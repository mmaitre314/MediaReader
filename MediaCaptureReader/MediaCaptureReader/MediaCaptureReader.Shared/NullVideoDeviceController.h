#pragma once

class NullVideoDeviceController WrlSealed :
    public Microsoft::WRL::RuntimeClass <
    MW::RuntimeClassFlags<MW::RuntimeClassType::WinRtClassicComMix>,
    AWMD::IVideoDeviceController,
    AWMD::IMediaDeviceController
    >
{
    InspectableClass(RuntimeClass_Windows_Media_Devices_VideoDeviceController, TrustLevel::BaseTrust);

public:

    NullVideoDeviceController();

    //
    // IVideoDeviceController
    //

    STDMETHOD(get_Brightness)(_COM_Outptr_ AWMD::IMediaDeviceControl **value) override
    {
        return OriginateError(E_NOTIMPL);
    }

    STDMETHOD(get_Contrast)(_COM_Outptr_ AWMD::IMediaDeviceControl **value) override
    {
        return OriginateError(E_NOTIMPL);
    }

    STDMETHOD(get_Hue)(_COM_Outptr_ AWMD::IMediaDeviceControl **value) override
    {
        return OriginateError(E_NOTIMPL);
    }

    STDMETHOD(get_WhiteBalance)(_COM_Outptr_ AWMD::IMediaDeviceControl **value) override
    {
        return OriginateError(E_NOTIMPL);
    }

    STDMETHOD(get_BacklightCompensation)(_COM_Outptr_ AWMD::IMediaDeviceControl **value) override
    {
        return OriginateError(E_NOTIMPL);
    }

    STDMETHOD(get_Pan)(_COM_Outptr_ AWMD::IMediaDeviceControl **value) override
    {
        return OriginateError(E_NOTIMPL);
    }

    STDMETHOD(get_Tilt)(_COM_Outptr_ AWMD::IMediaDeviceControl **value) override
    {
        return OriginateError(E_NOTIMPL);
    }

    STDMETHOD(get_Zoom)(_COM_Outptr_ AWMD::IMediaDeviceControl **value) override
    {
        return OriginateError(E_NOTIMPL);
    }

    STDMETHOD(get_Roll)(_COM_Outptr_ AWMD::IMediaDeviceControl **value) override
    {
        return OriginateError(E_NOTIMPL);
    }

    STDMETHOD(get_Exposure)(_COM_Outptr_ AWMD::IMediaDeviceControl **value) override
    {
        return OriginateError(E_NOTIMPL);
    }

    STDMETHOD(get_Focus)(_COM_Outptr_ AWMD::IMediaDeviceControl **value) override
    {
        return OriginateError(E_NOTIMPL);
    }

    STDMETHOD(TrySetPowerlineFrequency)(_In_ AWMC::PowerlineFrequency value, _Out_ boolean *succeeded) override
    {
        return OriginateError(E_NOTIMPL);
    }

    STDMETHOD(TryGetPowerlineFrequency)(_Out_ AWMC::PowerlineFrequency *value, _Out_ boolean *succeeded) override
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

    virtual ~NullVideoDeviceController();
};

