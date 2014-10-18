#include "pch.h"
#include "MediaGraphicsDevice.h"

using namespace MediaCaptureReader;
using namespace Microsoft::WRL;
using namespace Platform;

MediaGraphicsDevice^ MediaGraphicsDevice::CreateFromMediaCapture(Windows::Media::Capture::MediaCapture^ capture)
{
    CHKNULL(capture);

    auto device = ref new MediaGraphicsDevice();

    ComPtr<IAdvancedMediaCapture> advancedCapture;
    ComPtr<IAdvancedMediaCaptureSettings> settings;
    CHK(((IUnknown*)capture)->QueryInterface(IID_PPV_ARGS(&advancedCapture)));
    CHK(advancedCapture->GetAdvancedMediaCaptureSettings(&settings));
    CHK(settings->GetDirectxDeviceManager(&device->_deviceManager));

    return device;
}

MediaGraphicsDevice::~MediaGraphicsDevice()
{
}