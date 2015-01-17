#include "pch.h"
#include "MediaGraphicsDevice.h"

using namespace MediaCaptureReader;
using namespace Microsoft::WRL;
using namespace Platform;

MediaGraphicsDevice::MediaGraphicsDevice()
    : _deviceResetToken(0)
{
    CHK(MFCreateDXGIDeviceManager(&_deviceResetToken, &_deviceManager));

    D3D_FEATURE_LEVEL level;
    static const D3D_FEATURE_LEVEL levels[] =
    {
        D3D_FEATURE_LEVEL_11_1,
        D3D_FEATURE_LEVEL_11_0,
        D3D_FEATURE_LEVEL_10_1,
        D3D_FEATURE_LEVEL_10_0,
        D3D_FEATURE_LEVEL_9_3,
        D3D_FEATURE_LEVEL_9_2,
        D3D_FEATURE_LEVEL_9_1
    };

    ComPtr<ID3D11Device> device;
    CHK(D3D11CreateDevice(
        nullptr,
        D3D_DRIVER_TYPE_HARDWARE,
        nullptr,
        D3D11_CREATE_DEVICE_VIDEO_SUPPORT | D3D11_CREATE_DEVICE_BGRA_SUPPORT, // DEVICE_VIDEO needed for MF, BGRA for SurfaceImageSource
        levels,
        ARRAYSIZE(levels),
        D3D11_SDK_VERSION,
        &device,
        &level,
        nullptr
        ));

    As<ID3D10Multithread>(device)->SetMultithreadProtected(true);

    CHK(_deviceManager->ResetDevice(device.Get(), _deviceResetToken));
}

MediaGraphicsDevice::MediaGraphicsDevice(_In_ const MW::ComPtr<IMFDXGIDeviceManager>& deviceManager)
    : _deviceResetToken(0)
    , _deviceManager(deviceManager)
{
}

MediaGraphicsDevice^ MediaGraphicsDevice::CreateFromMediaCapture(_In_ Windows::Media::Capture::MediaCapture^ capture)
{
    CHKNULL(capture);

    ComPtr<IAdvancedMediaCapture> advancedCapture;
    ComPtr<IAdvancedMediaCaptureSettings> settings;
    ComPtr<IMFDXGIDeviceManager> deviceManager;
    CHK(((IUnknown*)capture)->QueryInterface(IID_PPV_ARGS(&advancedCapture)));
    CHK(advancedCapture->GetAdvancedMediaCaptureSettings(&settings));
    CHK(settings->GetDirectxDeviceManager(&deviceManager));

    return ref new MediaGraphicsDevice(deviceManager);
}

MediaGraphicsDevice::~MediaGraphicsDevice()
{
}