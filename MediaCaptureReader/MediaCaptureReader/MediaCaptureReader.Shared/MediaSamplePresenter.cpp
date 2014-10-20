#include "pch.h"
#include "MediaGraphicsDevice.h"
#include "MediaSample.h"
#include "MediaSamplePresenter.h"

using namespace MediaCaptureReader;
using namespace Microsoft::WRL;
using namespace Platform;
using namespace Windows::UI::Xaml::Media::Imaging;
using namespace Windows::UI::Xaml::Controls;

MediaSamplePresenter^ MediaSamplePresenter::CreateFromSurfaceImageSource(
    SurfaceImageSource^ image,
    MediaGraphicsDevice^ device
    )
{
    CHKNULL(image);
    CHKNULL(device);

    auto presenter = ref new MediaSamplePresenter();

    // Get the DXGI device
    HANDLE deviceHandle = nullptr;
    ComPtr<IDXGIDevice> dxgiDevice;
    ComPtr<IMFDXGIDeviceManager> deviceManager = device->GetDeviceManager();
    CHK(deviceManager->OpenDeviceHandle(&deviceHandle));
    HRESULT hr = deviceManager->GetVideoService(deviceHandle, IID_PPV_ARGS(&dxgiDevice));
    CHK(deviceManager->CloseDeviceHandle(deviceHandle));
    CHK(hr);

    // Initialize SurfaceImageSource
    CHK(((IUnknown*)image)->QueryInterface(IID_PPV_ARGS(&presenter->_image)));
    CHK(presenter->_image->SetDevice(dxgiDevice.Get()));

    return presenter;
}

MediaSamplePresenter^ MediaSamplePresenter::CreateFromSwapChainPanel(
    SwapChainPanel^ panel,
    MediaGraphicsDevice^ device,
    int width,
    int height
    )
{
    CHKNULL(panel);
    CHKNULL(device);

    auto presenter = ref new MediaSamplePresenter();

    // Get the DXGI device
    HANDLE deviceHandle = nullptr;
    ComPtr<IDXGIDevice> dxgiDevice;
    ComPtr<IMFDXGIDeviceManager> deviceManager = device->GetDeviceManager();
    CHK(deviceManager->OpenDeviceHandle(&deviceHandle));
    HRESULT hr = deviceManager->GetVideoService(deviceHandle, IID_PPV_ARGS(&dxgiDevice));
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
    CHK(dxgiDevice->GetAdapter(&dxgiAdapter));
    CHK(dxgiAdapter->GetParent(IID_PPV_ARGS(&dxgiFactory)));
    CHK(dxgiFactory->CreateSwapChainForComposition(
        dxgiDevice.Get(),
        &swapChainDesc,
        nullptr,
        &presenter->_swapChain
        ));

    // Initialize SwapChainPanel
    CHK(((IUnknown*)panel)->QueryInterface(IID_PPV_ARGS(&presenter->_panel)));
    CHK(presenter->_panel->SetSwapChain(presenter->_swapChain.Get()));

    return presenter;
}

MediaSamplePresenter::~MediaSamplePresenter()
{
}

void MediaSamplePresenter::Present(MediaSample^ sample)
{
    CHKNULL(sample);

    ComPtr<IMFMediaBuffer> buffer;
    CHK(sample->GetSample()->GetBufferByIndex(0, &buffer));
    auto dxgiBuffer = As<IMFDXGIBuffer>(buffer);

    if (_image != nullptr)
    {
        _PresentToSurfaceImageSource(dxgiBuffer);
    }
    else
    {
        _PresentToSwapChainPanel(dxgiBuffer);
    }
}

void MediaSamplePresenter::_PresentToSurfaceImageSource(const ComPtr<IMFDXGIBuffer>& dxgiBuffer)
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

void MediaSamplePresenter::_PresentToSwapChainPanel(const ComPtr<IMFDXGIBuffer>& dxgiBuffer)
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
