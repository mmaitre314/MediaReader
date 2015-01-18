#include "pch.h"
#include "MediaGraphicsDevice.h"
#include "MediaSample.h"
#include "MediaSample2DPresenter.h"

using namespace MediaCaptureReader;
using namespace Microsoft::WRL;
using namespace Platform;
using namespace Windows::UI::Xaml::Media::Imaging;
using namespace Windows::UI::Xaml::Controls;

MediaSample2DPresenter^ MediaSample2DPresenter::CreateFromSurfaceImageSource(
    _In_ SurfaceImageSource^ image,
    _In_ MediaGraphicsDevice^ device,
    _In_ int width,
    _In_ int height
    )
{
    CHKNULL(image);
    CHKNULL(device);

    auto presenter = ref new MediaSample2DPresenter(width, height);

    // Get the DXGI device
    HANDLE deviceHandle = nullptr;
    ComPtr<IMFDXGIDeviceManager> deviceManager = device->GetDeviceManager();
    CHK(deviceManager->OpenDeviceHandle(&deviceHandle));
    HRESULT hr = deviceManager->GetVideoService(deviceHandle, IID_PPV_ARGS(&presenter->_device));
    CHK(deviceManager->CloseDeviceHandle(deviceHandle));
    CHK(hr);

    // Initialize SurfaceImageSource
    CHK(((IUnknown*)image)->QueryInterface(IID_PPV_ARGS(&presenter->_image)));
    CHK(presenter->_image->SetDevice(presenter->_device.Get()));

    return presenter;
}

MediaSample2DPresenter^ MediaSample2DPresenter::CreateFromSwapChainPanel(
    _In_ SwapChainPanel^ panel,
    _In_ MediaGraphicsDevice^ device,
    _In_ int width,
    _In_ int height
    )
{
    CHKNULL(panel);
    CHKNULL(device);

    auto presenter = ref new MediaSample2DPresenter(width, height);

    // Get the DXGI device
    HANDLE deviceHandle = nullptr;
    ComPtr<IMFDXGIDeviceManager> deviceManager = device->GetDeviceManager();
    CHK(deviceManager->OpenDeviceHandle(&deviceHandle));
    HRESULT hr = deviceManager->GetVideoService(deviceHandle, IID_PPV_ARGS(&presenter->_device));
    CHK(deviceManager->CloseDeviceHandle(deviceHandle));
    CHK(hr);

    // Swap chain parameters
    DXGI_SWAP_CHAIN_DESC1 swapChainDesc = {};
    swapChainDesc.Width = (unsigned int)width;
    swapChainDesc.Height = (unsigned int)height;
    swapChainDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
    swapChainDesc.Stereo = false;
    swapChainDesc.SampleDesc.Count = 1;
    swapChainDesc.SampleDesc.Quality = 0;
    swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    swapChainDesc.BufferCount = 2;
    swapChainDesc.Scaling = DXGI_SCALING_STRETCH;
    swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_SEQUENTIAL;
    swapChainDesc.Flags = 0;

    // Create the swap chain
    ComPtr<IDXGIAdapter> dxgiAdapter;
    ComPtr<IDXGIFactory2> dxgiFactory;
    CHK(presenter->_device->GetAdapter(&dxgiAdapter));
    CHK(dxgiAdapter->GetParent(IID_PPV_ARGS(&dxgiFactory)));
    CHK(dxgiFactory->CreateSwapChainForComposition(
        presenter->_device.Get(),
        &swapChainDesc,
        nullptr,
        &presenter->_swapChain
        ));

    // Initialize SwapChainPanel
    CHK(((IUnknown*)panel)->QueryInterface(IID_PPV_ARGS(&presenter->_panel)));
    CHK(presenter->_panel->SetSwapChain(presenter->_swapChain.Get()));

    return presenter;
}

MediaSample2DPresenter::~MediaSample2DPresenter()
{
}

void MediaSample2DPresenter::Present(MediaSample2D^ sample)
{
    CHKNULL(sample);

    if (sample->Format != MediaSample2DFormat::Bgra8)
    {
        throw ref new InvalidArgumentException(L"Only Bgra8 supported");
    }

    ComPtr<IMFMediaBuffer> buffer;
    CHK(sample->GetSample()->GetBufferByIndex(0, &buffer));

    // Get the MF DXGI buffer, make a copy if the buffer is not yet a DXGI one
    ComPtr<IMFDXGIBuffer> bufferDxgi;
    if (FAILED(buffer.As(&bufferDxgi)))
    {
        auto buffer2D = As<IMF2DBuffer2>(buffer);

        D3D11_TEXTURE2D_DESC desc = {};
        desc.Width = _width;
        desc.Height = _height;
        desc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
        desc.Usage = D3D11_USAGE_STAGING;
        desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
        desc.BindFlags = 0;
        desc.MiscFlags = 0;
        desc.ArraySize = 1;
        desc.MipLevels = 1;
        desc.SampleDesc.Count = 1;
        desc.SampleDesc.Quality = 0;

        ComPtr<ID3D11Texture2D> texture;
        CHK(As<ID3D11Device>(_device)->CreateTexture2D(&desc, nullptr, &texture));

        ComPtr<IMFMediaBuffer> bufferCopy;
        CHK(MFCreateDXGISurfaceBuffer(__uuidof(texture), texture.Get(), 0, /*fBottomUpWhenLinear*/false, &bufferCopy));
        CHK(buffer2D->Copy2DTo(As<IMF2DBuffer2>(bufferCopy).Get()));

        bufferDxgi = As<IMFDXGIBuffer>(bufferCopy);
    }

    if (_image != nullptr)
    {
        _PresentToSurfaceImageSource(bufferDxgi);
    }
    else
    {
        _PresentToSwapChainPanel(bufferDxgi);
    }
}

void MediaSample2DPresenter::_PresentToSurfaceImageSource(const ComPtr<IMFDXGIBuffer>& dxgiBuffer)
{
    ComPtr<ID3D11Texture2D> texture;
    unsigned int subresource;
    CHK(dxgiBuffer->GetResource(IID_PPV_ARGS(&texture)));
    CHK(dxgiBuffer->GetSubresourceIndex(&subresource));

    D3D11_TEXTURE2D_DESC desc;
    texture->GetDesc(&desc);

    RECT updateRect = { 0, 0, (long)desc.Width, (long)desc.Height };
    POINT offset;
    ComPtr<IDXGISurface> surfaceDst;
    CHK(_image->BeginDraw(updateRect, &surfaceDst, &offset));
    auto textureDst = As<ID3D11Texture2D>(surfaceDst);
    
    ComPtr<ID3D11Device> device;
    ComPtr<ID3D11DeviceContext> context;
    texture->GetDevice(&device);
    device->GetImmediateContext(&context);

    context->CopySubresourceRegion(textureDst.Get(), 0, offset.x, offset.y, 0, texture.Get(), subresource, nullptr);

    CHK(_image->EndDraw());
}

void MediaSample2DPresenter::_PresentToSwapChainPanel(const ComPtr<IMFDXGIBuffer>& dxgiBuffer)
{
    ComPtr<ID3D11Texture2D> texture;
    unsigned int subresource;
    CHK(dxgiBuffer->GetResource(IID_PPV_ARGS(&texture)));
    CHK(dxgiBuffer->GetSubresourceIndex(&subresource));

    ComPtr<ID3D11Texture2D> textureDst;
    CHK(_swapChain->GetBuffer(0, IID_PPV_ARGS(&textureDst)));

    ComPtr<ID3D11Device> device;
    ComPtr<ID3D11DeviceContext> context;
    texture->GetDevice(&device);
    device->GetImmediateContext(&context);

    context->CopySubresourceRegion(textureDst.Get(), 0, 0, 0, 0, texture.Get(), subresource, nullptr);

    CHK(_swapChain->Present(1, 0));
}
