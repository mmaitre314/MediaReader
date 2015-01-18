#pragma once

class NullAudioDeviceController;
class NullVideoDeviceController;
class NullMediaCaptureSettings;

class NullMediaCaptureImpl WrlSealed :
    public Microsoft::WRL::RuntimeClass <
    MW::RuntimeClassFlags<MW::RuntimeClassType::WinRtClassicComMix>,
    AWMC::IMediaCapture,
    AWMC::IMediaCaptureVideoPreview,
    MW::CloakedIid<IAdvancedMediaCapture>,
    MW::CloakedIid<IAdvancedMediaCaptureSettings>
    >
{
    InspectableClass(RuntimeClass_Windows_Media_Capture_MediaCapture, TrustLevel::BaseTrust);

public:

    NullMediaCaptureImpl();

    //
    // IMediaCapture
    //

    STDMETHOD(InitializeAsync)(_COM_Outptr_ AWF::IAsyncAction **asyncInfo) override;

    STDMETHOD(InitializeWithSettingsAsync)(
        _In_ AWMC::IMediaCaptureInitializationSettings *mediaCaptureInitializationSettings,
        _COM_Outptr_ AWF::IAsyncAction **asyncInfo
        ) override;

    STDMETHOD(StartRecordToStorageFileAsync)(
        _In_ AWMMp::IMediaEncodingProfile *encodingProfile,
        _In_ ABI::Windows::Storage::IStorageFile *file,
        _COM_Outptr_ AWF::IAsyncAction **asyncInfo
        ) override
    {
        return OriginateError(E_NOTIMPL);
    }

    STDMETHOD(StartRecordToStreamAsync)(
        _In_ AWMMp::IMediaEncodingProfile *encodingProfile,
        _In_ AWSS::IRandomAccessStream *stream,
        _COM_Outptr_ AWF::IAsyncAction **asyncInfo
        ) override
    {
        return OriginateError(E_NOTIMPL);
    }

    STDMETHOD(StartRecordToCustomSinkAsync)(
        _In_ AWMMp::IMediaEncodingProfile *encodingProfile,
        _In_ AWM::IMediaExtension *customMediaSink,
        _COM_Outptr_ AWF::IAsyncAction **asyncInfo
        ) override
    {
        return OriginateError(E_NOTIMPL);
    }

    STDMETHOD(StartRecordToCustomSinkIdAsync)(
        _In_ AWMMp::IMediaEncodingProfile *encodingProfile,
        _In_ HSTRING customSinkActivationId,
        _In_ AWFC::IPropertySet *customSinkSettings,
        _COM_Outptr_ AWF::IAsyncAction **asyncInfo
        ) override
    {
        return OriginateError(E_NOTIMPL);
    }

    STDMETHOD(StopRecordAsync)(
        _COM_Outptr_ AWF::IAsyncAction **asyncInfo
        ) override
    {
        return OriginateError(E_NOTIMPL);
    }

    STDMETHOD(CapturePhotoToStorageFileAsync)(
        _In_ AWMMp::IImageEncodingProperties *type,
        _In_ ABI::Windows::Storage::IStorageFile *file,
        _COM_Outptr_ AWF::IAsyncAction **asyncInfo
        ) override
    {
        return OriginateError(E_NOTIMPL);
    }

    STDMETHOD(CapturePhotoToStreamAsync)(
        _In_ AWMMp::IImageEncodingProperties *type,
        _In_ ABI::Windows::Storage::Streams::IRandomAccessStream *stream,
        _COM_Outptr_ AWF::IAsyncAction **asyncInfo
        ) override
    {
        return OriginateError(E_NOTIMPL);
    }

    STDMETHOD(AddEffectAsync)(
        _In_ AWMC::MediaStreamType mediaStreamType,
        _In_ HSTRING effectActivationID,
        _In_ AWFC::IPropertySet *effectSettings,
        _COM_Outptr_ AWF::IAsyncAction **asyncInfo
        ) override
    {
        return OriginateError(E_NOTIMPL);
    }

    STDMETHOD(ClearEffectsAsync)(
        _In_ AWMC::MediaStreamType mediaStreamType,
        _COM_Outptr_ AWF::IAsyncAction **asyncInfo
        ) override
    {
        return OriginateError(E_NOTIMPL);
    }

    STDMETHOD(SetEncoderProperty)(
        _In_ AWMC::MediaStreamType mediaStreamType,
        _In_ GUID propertyId,
        _In_ IInspectable *propertyValue
        ) override
    {
        return OriginateError(E_NOTIMPL);
    }

    STDMETHOD(GetEncoderProperty)(
        _In_ AWMC::MediaStreamType mediaStreamType,
        _In_ GUID propertyId,
        _COM_Outptr_ IInspectable **propertyValue
        ) override
    {
        return OriginateError(E_NOTIMPL);
    }

    STDMETHOD(add_Failed)(
        _In_ AWMC::IMediaCaptureFailedEventHandler *errorEventHandler,
        _Out_ EventRegistrationToken *eventCookie
        ) override
    {
        return OriginateError(E_NOTIMPL);
    }

    STDMETHOD(remove_Failed)(_In_ EventRegistrationToken eventCookie) override
    {
        return OriginateError(E_NOTIMPL);
    }

    STDMETHOD(add_RecordLimitationExceeded)(
        _In_ AWMC::IRecordLimitationExceededEventHandler *recordLimitationExceededEventHandler,
        _Out_ EventRegistrationToken *eventCookie
        ) override
    {
        return OriginateError(E_NOTIMPL);
    }

    STDMETHOD(remove_RecordLimitationExceeded)(_In_ EventRegistrationToken eventCookie) override
    {
        return OriginateError(E_NOTIMPL);
    }

    STDMETHOD(get_MediaCaptureSettings)(_COM_Outptr_ AWMC::IMediaCaptureSettings **value) override;

    STDMETHOD(get_AudioDeviceController)(_COM_Outptr_ AWMD::IAudioDeviceController **value) override;

    STDMETHOD(get_VideoDeviceController)(_COM_Outptr_ AWMD::IVideoDeviceController **value) override;

    STDMETHOD(SetPreviewMirroring)(_In_ boolean value) override
    {
        return OriginateError(E_NOTIMPL);
    }

    STDMETHOD(GetPreviewMirroring)(_Out_ boolean *value) override
    {
        return OriginateError(E_NOTIMPL);
    }

    STDMETHOD(SetPreviewRotation)(_In_ AWMC::VideoRotation value) override
    {
        return OriginateError(E_NOTIMPL);
    }

    STDMETHOD(GetPreviewRotation)(_Out_ AWMC::VideoRotation *value) override
    {
        return OriginateError(E_NOTIMPL);
    }

    STDMETHOD(SetRecordRotation)(_In_ AWMC::VideoRotation value) override
    {
        return OriginateError(E_NOTIMPL);
    }

    STDMETHOD(GetRecordRotation)(_Out_ AWMC::VideoRotation *value) override
    {
        return OriginateError(E_NOTIMPL);
    }

    //
    // IMediaCaptureVideoPreview
    //

    STDMETHOD(StartPreviewAsync)(_COM_Outptr_ AWF::IAsyncAction **asyncInfo) override
    {
        return OriginateError(E_NOTIMPL);
    }

    STDMETHOD(StartPreviewToCustomSinkAsync)(
        _In_ AWMMp::IMediaEncodingProfile *encodingProfile,
        _In_ AWM::IMediaExtension *customMediaSink,
        _COM_Outptr_ AWF::IAsyncAction **asyncInfo
        ) override;

    STDMETHOD(StartPreviewToCustomSinkIdAsync)(
        _In_ AWMMp::IMediaEncodingProfile *encodingProfile,
        _In_ HSTRING customSinkActivationId,
        _In_ AWFC::IPropertySet *customSinkSettings,
        _COM_Outptr_ AWF::IAsyncAction **asyncInfo
        ) override
    {
        return OriginateError(E_NOTIMPL);
    }

    STDMETHOD(StopPreviewAsync)(_COM_Outptr_ AWF::IAsyncAction **asyncInfo) override;

    //
    // IAdvancedMediaCapture
    //

    STDMETHOD(GetAdvancedMediaCaptureSettings)(_COM_Outptr_ IAdvancedMediaCaptureSettings **value) override;

    // 
    // IAdvancedMediaCaptureSettings
    //

    STDMETHOD(GetDirectxDeviceManager)(_COM_Outptr_ IMFDXGIDeviceManager **value) override;

private:

    virtual ~NullMediaCaptureImpl();

    void _HandlePreviewSinkRequests();
    MW::ComPtr<IMFSample> _CreateVideoSampleBgra8(_In_ unsigned int width, _In_ unsigned int height, _In_ MFTIME time);
    MW::ComPtr<IMFSample> _CreateVideoSampleNv12(_In_ unsigned int width, _In_ unsigned int height, _In_ MFTIME time);

    MW::ComPtr<IMFDXGIDeviceManager> _deviceManager;
    MW::ComPtr<NullAudioDeviceController> _audioDeviceController;
    MW::ComPtr<NullVideoDeviceController> _videoDeviceController;
    MW::ComPtr<NullMediaCaptureSettings> _mediaCaptureSettings;
    MW::ComPtr<IMFStreamSink> _audioStreamSink;
    MW::ComPtr<IMFStreamSink> _videoStreamSink;

    MFTIME _previewStartTime;
    unsigned int _previewFourCC;

    MWW::SRWLock _lock;
};

