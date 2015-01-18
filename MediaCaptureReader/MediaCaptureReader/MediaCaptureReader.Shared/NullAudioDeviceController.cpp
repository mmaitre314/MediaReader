#include "pch.h"
#include "NullAudioDeviceController.h"

using namespace ABI::Windows::Media::Capture;
using namespace ABI::Windows::Media::MediaProperties;

NullAudioDeviceController::NullAudioDeviceController()
{
}

NullAudioDeviceController::~NullAudioDeviceController()
{
}

STDMETHODIMP NullAudioDeviceController::GetMediaStreamProperties(
    _In_ MediaStreamType mediaStreamType,
    _COM_Outptr_ AWMMp::IMediaEncodingProperties **value
    )
{
    return ExceptionBoundary([=]()
    {
        *value = nullptr;

        if (mediaStreamType != MediaStreamType_Audio)
        {
            CHK(OriginateError(E_INVALIDARG, L"mediaStreamType"));
        }

        auto props = Windows::Media::MediaProperties::AudioEncodingProperties::CreatePcm(44100, 1, 16);

        *value = reinterpret_cast<IMediaEncodingProperties*>(
            static_cast<Windows::Media::MediaProperties::IMediaEncodingProperties^>(props)
            );
        (*value)->AddRef();
    });
}
