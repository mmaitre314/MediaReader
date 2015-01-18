#pragma once

class NullAudioDeviceController WrlSealed :
    public Microsoft::WRL::RuntimeClass <
    MW::RuntimeClassFlags<MW::RuntimeClassType::WinRtClassicComMix>,
    AWMD::IAudioDeviceController,
    AWMD::IMediaDeviceController
    >
{
    InspectableClass(RuntimeClass_Windows_Media_Devices_AudioDeviceController, TrustLevel::BaseTrust);

public:

    NullAudioDeviceController();

    //
    // IAudioDeviceController
    //

    STDMETHOD(put_Muted)(_In_ boolean value) override
    {
        return OriginateError(E_NOTIMPL);
    }

    STDMETHOD(get_Muted)(_Out_ boolean *value) override
    {
        return OriginateError(E_NOTIMPL);
    }

    STDMETHOD(put_VolumePercent)(_In_ FLOAT value) override
    {
        return OriginateError(E_NOTIMPL);
    }

    STDMETHOD(get_VolumePercent)(_Out_ FLOAT *value) override
    {
        return OriginateError(E_NOTIMPL);
    }

    //
    // IMediaDeviceController
    //

    STDMETHOD(GetAvailableMediaStreamProperties)(
        _In_ AWMC::MediaStreamType mediaStreamType,
        _COM_Outptr_ AWFC::IVectorView<AWMMp::IMediaEncodingProperties*> **value
        ) override
    {
        return OriginateError(E_NOTIMPL);
    }

    STDMETHOD(GetMediaStreamProperties)(_In_ AWMC::MediaStreamType mediaStreamType, _COM_Outptr_ AWMMp::IMediaEncodingProperties **value) override;

    STDMETHOD(SetMediaStreamPropertiesAsync)(
        _In_ AWMC::MediaStreamType mediaStreamType,
        _In_ AWMMp::IMediaEncodingProperties *mediaEncodingProperties,
        _COM_Outptr_ AWF::IAsyncAction **asyncInfo
        ) override
    {
        return OriginateError(E_NOTIMPL);
    }

private:

    virtual ~NullAudioDeviceController();
};

