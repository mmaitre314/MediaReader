#include "pch.h"
#include "NullVideoDeviceController.h"

using namespace ABI::Windows::Media::Capture;
using namespace ABI::Windows::Media::MediaProperties;

NullVideoDeviceController::NullVideoDeviceController()
{
}

NullVideoDeviceController::~NullVideoDeviceController()
{
}

STDMETHODIMP NullVideoDeviceController::GetMediaStreamProperties(
    _In_ MediaStreamType mediaStreamType, 
    _COM_Outptr_ AWMMp::IMediaEncodingProperties **value
    )
{
    return ExceptionBoundary([=]()
    {
        *value = nullptr;

        if ((mediaStreamType == MediaStreamType_VideoPreview) ||
            (mediaStreamType == MediaStreamType_VideoRecord))
        {
            auto props = Windows::Media::MediaProperties::VideoEncodingProperties::CreateUncompressed(
                Windows::Media::MediaProperties::MediaEncodingSubtypes::Nv12, 
                320, 
                240
                );
            props->FrameRate->Numerator = 30;
            props->FrameRate->Denominator = 1;

            *value = reinterpret_cast<IMediaEncodingProperties*>(
                static_cast<Windows::Media::MediaProperties::IMediaEncodingProperties^>(props)
                );
            (*value)->AddRef();
        }
        else
        {
            CHK(OriginateError(E_NOTIMPL));
        }
    });
}
