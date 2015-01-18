#pragma once

class NullMediaCaptureSettings WrlSealed :
    public Microsoft::WRL::RuntimeClass <
    AWMC::IMediaCaptureSettings
    >
{
    InspectableClass(RuntimeClass_Windows_Media_Capture_MediaCaptureSettings, TrustLevel::BaseTrust);

public:

    NullMediaCaptureSettings();

    void InitializeStreamingCaptureMode(_In_ AWMC::StreamingCaptureMode mode)
    {
        _streamingCaptureMode = mode;
    }

    //
    // IMediaCaptureSettings
    //

    STDMETHOD(get_AudioDeviceId)(_Out_ HSTRING *value) override
    {
        if ((_streamingCaptureMode == AWMC::StreamingCaptureMode_AudioAndVideo) ||
            (_streamingCaptureMode == AWMC::StreamingCaptureMode_Audio))
        {
            return WindowsCreateString(L"AudioDeviceId", ARRAYSIZE(L"AudioDeviceId") - 1, value);
        }
        else
        {
            *value = nullptr;
            return S_OK;
        }
    }

    STDMETHOD(get_VideoDeviceId)(_Out_ HSTRING *value) override
    {
        if ((_streamingCaptureMode == AWMC::StreamingCaptureMode_AudioAndVideo) ||
            (_streamingCaptureMode == AWMC::StreamingCaptureMode_Video))
        {
            return WindowsCreateString(L"VideoDeviceId", ARRAYSIZE(L"VideoDeviceId") - 1, value);
        }
        else
        {
            *value = nullptr;
            return S_OK;
        }
    }

    STDMETHOD(get_StreamingCaptureMode)(_Out_ AWMC::StreamingCaptureMode *value) override
    {
        *value = _streamingCaptureMode;
        return S_OK;
    }

    STDMETHOD(get_PhotoCaptureSource)(_Out_ AWMC::PhotoCaptureSource *value) override
    {
        *value = AWMC::PhotoCaptureSource_VideoPreview;
        return S_OK;
    }

    STDMETHOD(get_VideoDeviceCharacteristic)(_Out_ AWMC::VideoDeviceCharacteristic *value) override
    {
        *value = AWMC::VideoDeviceCharacteristic_AllStreamsIdentical;
        return S_OK;
    }

private:

    AWMC::StreamingCaptureMode _streamingCaptureMode;

    virtual ~NullMediaCaptureSettings();
};
