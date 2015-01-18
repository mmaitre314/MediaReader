#include "pch.h"
#include "NullMediaCaptureImpl.h"
#include "NullAudioDeviceController.h"
#include "NullVideoDeviceController.h"
#include "NullMediaCaptureSettings.h"
#include "MediaGraphicsDevice.h"

using namespace ABI::Windows::Foundation;
using namespace ABI::Windows::Foundation::Collections;
using namespace ABI::Windows::Media;
using namespace ABI::Windows::Media::Capture;
using namespace ABI::Windows::Media::Devices;
using namespace ABI::Windows::Media::MediaProperties;
using namespace concurrency;
using namespace MediaCaptureReader;
using namespace Microsoft::WRL;

NullMediaCaptureImpl::NullMediaCaptureImpl()
    : _previewStartTime(0)
    , _previewFourCC(0)
{
    _deviceManager = (ref new MediaGraphicsDevice())->GetDeviceManager();

    _audioDeviceController = Make<NullAudioDeviceController>();
    CHKOOM(_audioDeviceController);

    _videoDeviceController = Make<NullVideoDeviceController>();
    CHKOOM(_videoDeviceController);

    _mediaCaptureSettings = Make<NullMediaCaptureSettings>();
    CHKOOM(_mediaCaptureSettings);
}

NullMediaCaptureImpl::~NullMediaCaptureImpl()
{
}

STDMETHODIMP NullMediaCaptureImpl::InitializeAsync(_COM_Outptr_ IAsyncAction **asyncInfo)
{
    return ExceptionBoundary([=]()
    {
        auto lock = _lock.LockExclusive();
        *asyncInfo = nullptr;

        Windows::Foundation::IAsyncAction^ action = create_async([]()
        {
            return task_from_result();
        });

        *asyncInfo = reinterpret_cast<IAsyncAction*>(action);
        (*asyncInfo)->AddRef();
    });
}

STDMETHODIMP NullMediaCaptureImpl::InitializeWithSettingsAsync(
    _In_ IMediaCaptureInitializationSettings *mediaCaptureInitializationSettings,
    _COM_Outptr_ IAsyncAction **asyncInfo
    ) 
{
    return ExceptionBoundary([=]()
    {
        auto lock = _lock.LockExclusive();
        *asyncInfo = nullptr;

        CHKNULL(mediaCaptureInitializationSettings);

        StreamingCaptureMode mode;
        CHK(mediaCaptureInitializationSettings->get_StreamingCaptureMode(&mode));
        _mediaCaptureSettings->InitializeStreamingCaptureMode(mode);

        Windows::Foundation::IAsyncAction^ action = create_async([]()
        {
            return task_from_result();
        });

        *asyncInfo = reinterpret_cast<IAsyncAction*>(action);
        (*asyncInfo)->AddRef();
    });
}

STDMETHODIMP NullMediaCaptureImpl::GetAdvancedMediaCaptureSettings(_COM_Outptr_ IAdvancedMediaCaptureSettings **value)
{
    return ExceptionBoundary([=]()
    {
        *value = this;
        (*value)->AddRef();
    });
}

STDMETHODIMP NullMediaCaptureImpl::GetDirectxDeviceManager(_COM_Outptr_ IMFDXGIDeviceManager **value)
{
    return ExceptionBoundary([=]()
    {
        auto lock = _lock.LockShared();
        CHK(_deviceManager.CopyTo(value));
    });
}

STDMETHODIMP NullMediaCaptureImpl::get_AudioDeviceController(_COM_Outptr_ IAudioDeviceController **value)
{
    return ExceptionBoundary([=]()
    {
        auto lock = _lock.LockShared();
        CHK(_audioDeviceController.CopyTo(value));
    });
}

STDMETHODIMP NullMediaCaptureImpl::get_VideoDeviceController(_COM_Outptr_ IVideoDeviceController **value)
{
    return ExceptionBoundary([=]()
    {
        auto lock = _lock.LockShared();
        CHK(_videoDeviceController.CopyTo(value));
    });
}

STDMETHODIMP NullMediaCaptureImpl::get_MediaCaptureSettings(_COM_Outptr_ AWMC::IMediaCaptureSettings **value)
{
    return ExceptionBoundary([=]()
    {
        auto lock = _lock.LockShared();
        CHK(_mediaCaptureSettings.CopyTo(value));
    });
}

STDMETHODIMP NullMediaCaptureImpl::StartPreviewToCustomSinkAsync(
    _In_ IMediaEncodingProfile * /*encodingProfile*/,
    _In_ IMediaExtension *customMediaSink,
    _COM_Outptr_ IAsyncAction **asyncInfo
    )
{
    return ExceptionBoundary([=]()
    {
        auto lock = _lock.LockExclusive();
        *asyncInfo = nullptr;

        CHKNULL(customMediaSink);

        if ((_audioStreamSink != nullptr) || (_videoStreamSink != nullptr))
        {
            CHK(OriginateError(E_ILLEGAL_METHOD_CALL))
        }

        auto mediaSink = As<IMFMediaSink>(customMediaSink);

        unsigned long streamCount;
        CHK(mediaSink->GetStreamSinkCount(&streamCount));
        if ((streamCount == 0) || (streamCount > 2))
        {
            CHK(OriginateError(E_INVALIDARG, L"Invalid sink stream count"));
        }

        for (unsigned int streamIndex = 0; streamIndex < streamCount; streamIndex++)
        {
            ComPtr<IMFStreamSink> streamSink;
            ComPtr<IMFMediaTypeHandler> handler;
            ComPtr<IMFMediaType> type;
            CHK(mediaSink->GetStreamSinkByIndex(streamIndex, &streamSink));
            CHK(streamSink->GetMediaTypeHandler(&handler));
            CHK(handler->GetCurrentMediaType(&type));

            GUID majorType;
            GUID subType;
            CHK(type->GetGUID(MF_MT_MAJOR_TYPE, &majorType));
            CHK(type->GetGUID(MF_MT_SUBTYPE, &subType));

            if (majorType == MFMediaType_Audio)
            {
                if (_audioStreamSink != nullptr)
                {
                    CHK(OriginateError(E_INVALIDARG, L"Multiple audio streams on sink"));
                }

                if (subType != MFAudioFormat_PCM)
                {
                    CHK(OriginateError(E_INVALIDARG, L"Audio subtype"));
                }

                _audioStreamSink = streamSink;
            }
            else if (majorType == MFMediaType_Video)
            {
                if (_videoStreamSink != nullptr)
                {
                    CHK(OriginateError(E_INVALIDARG, L"Multiple video streams on sink"));
                }

                if ((subType != MFVideoFormat_ARGB32) && (subType != MFVideoFormat_NV12))
                {
                    CHK(OriginateError(E_INVALIDARG, L"Video subtype"));
                }

                _previewFourCC = subType.Data1;
                _videoStreamSink = streamSink;
            }
            else
            {
                CHK(OriginateError(E_INVALIDARG, L"Sink steam type"));
            }
        }

        _previewStartTime = MFGetSystemTime();

        ComPtr<NullMediaCaptureImpl> spThis(this);
        create_task([spThis]()
        {
            spThis->_HandlePreviewSinkRequests();
        });

        Windows::Foundation::IAsyncAction^ action = create_async([]()
        {
            return task_from_result();
        });

        *asyncInfo = reinterpret_cast<IAsyncAction*>(action);
        (*asyncInfo)->AddRef();
    });
}

STDMETHODIMP NullMediaCaptureImpl::StopPreviewAsync(_COM_Outptr_ IAsyncAction **asyncInfo)
{
    return ExceptionBoundary([=]()
    {
        auto lock = _lock.LockExclusive();
        *asyncInfo = nullptr;

        if ((_audioStreamSink == nullptr) && (_videoStreamSink == nullptr))
        {
            CHK(OriginateError(E_ILLEGAL_METHOD_CALL))
        }

        _audioStreamSink = nullptr;
        _videoStreamSink = nullptr;

        Windows::Foundation::IAsyncAction^ action = create_async([]()
        {
            return task_from_result();
        });

        *asyncInfo = reinterpret_cast<IAsyncAction*>(action);
        (*asyncInfo)->AddRef();
    });
}

void NullMediaCaptureImpl::_HandlePreviewSinkRequests()
{
    while (true)
    {
        ComPtr<IMFStreamSink> audioStreamSink;
        ComPtr<IMFStreamSink> videoStreamSink;
        MFTIME previewStartTime;
        {
            auto lock = _lock.LockExclusive();
            audioStreamSink = _audioStreamSink;
            videoStreamSink = _videoStreamSink;
            if ((audioStreamSink == nullptr) && (videoStreamSink == nullptr))
            {
                break;
            }

            previewStartTime = _previewStartTime;
        }

        // TODO: handle audio sample requests

        ComPtr<IMFMediaEvent> event;
        HRESULT hr = videoStreamSink->GetEvent(0, &event);
        if (hr == MF_E_SHUTDOWN)
        {
            break;
        }
        CHK(hr);

        MediaEventType eventType;
        CHK(event->GetType(&eventType));
        if (eventType != MEStreamSinkRequestSample)
        {
            CHK(OriginateError(E_UNEXPECTED));
        }

        ComPtr<IMFMediaTypeHandler> handler;
        ComPtr<IMFMediaType> mediaType;
        unsigned int width;
        unsigned int height;
        CHK(videoStreamSink->GetMediaTypeHandler(&handler));
        CHK(handler->GetCurrentMediaType(&mediaType));
        CHK(MFGetAttributeSize(mediaType.Get(), MF_MT_FRAME_SIZE, &width, &height));

        ComPtr<IMFSample> sample;
        if (_previewFourCC == MFVideoFormat_ARGB32.Data1)
        {
            sample = _CreateVideoSampleBgra8(width, height, MFGetSystemTime() - previewStartTime);
        }
        else if (_previewFourCC == MFVideoFormat_NV12.Data1)
        {
            sample = _CreateVideoSampleNv12(width, height, MFGetSystemTime() - previewStartTime);
        }
        else
        {
            assert(false);
            CHK(OriginateError(MF_E_UNEXPECTED));
        }

        hr = videoStreamSink->ProcessSample(sample.Get());
        if (hr == MF_E_SHUTDOWN)
        {
            break;
        }
        CHK(hr);
    }
}

ComPtr<IMFSample> NullMediaCaptureImpl::_CreateVideoSampleBgra8(_In_ unsigned int width, _In_ unsigned int height, _In_ MFTIME time)
{
    ComPtr<IMFMediaBuffer> buffer;
    CHK(MFCreate2DMediaBuffer(width, height, MFVideoFormat_ARGB32.Data1, /*fBottomUp*/false, &buffer));

    // Fill buffer with some pattern
    auto buffer2D = As<IMF2DBuffer2>(buffer);
    unsigned char *data = nullptr;
    unsigned char *bufferStart = nullptr;
    long pitch;
    unsigned long length;
    CHK(buffer2D->Lock2DSize(MF2DBuffer_LockFlags_Write, &data, &pitch, &bufferStart, &length));
    for (unsigned int i = 0; i < height; i++)
    {
        for (unsigned int j = 0; j < width; j++)
        {
            data[i * pitch + 4 * j + 0] = (unsigned char)i;
            data[i * pitch + 4 * j + 1] = (unsigned char)j;
            data[i * pitch + 4 * j + 2] = (unsigned char)((100 * time) / 10000000);
            data[i * pitch + 4 * j + 3] = 255;
        }
    }
    CHK(buffer2D->Unlock2D());

    CHK(buffer->SetCurrentLength(width * height * 4))

    ComPtr<IMFSample> sample;
    CHK(MFCreateSample(&sample));
    CHK(sample->SetSampleTime(time));
    CHK(sample->AddBuffer(buffer.Get()));

    return sample;
}

ComPtr<IMFSample> NullMediaCaptureImpl::_CreateVideoSampleNv12(_In_ unsigned int width, _In_ unsigned int height, _In_ MFTIME time)
{
    ComPtr<IMFMediaBuffer> buffer;
    CHK(MFCreate2DMediaBuffer(width, height, MFVideoFormat_NV12.Data1, /*fBottomUp*/false, &buffer));

    // Fill buffer with some pattern
    auto buffer2D = As<IMF2DBuffer2>(buffer);
    unsigned char *data = nullptr;
    unsigned char *bufferStart = nullptr;
    long pitch;
    unsigned long length;
    CHK(buffer2D->Lock2DSize(MF2DBuffer_LockFlags_Write, &data, &pitch, &bufferStart, &length));
    for (unsigned int i = 0; i < height; i++) // Y
    {
        for (unsigned int j = 0; j < width; j++)
        {
            data[i * pitch + j] = (unsigned char)((100 * time) / 10000000 + 128);
        }
    }
    for (unsigned int i = 0; i < height / 2; i++) // UV
    {
        for (unsigned int j = 0; j < width / 2; j++)
        {
            data[height * pitch + i * pitch + 2 * j + 0] = (unsigned char)i;
            data[height * pitch + i * pitch + 2 * j + 1] = (unsigned char)j;
        }
    }
    CHK(buffer2D->Unlock2D());

    CHK(buffer->SetCurrentLength((width * height * 3) / 2));

    ComPtr<IMFSample> sample;
    CHK(MFCreateSample(&sample));
    CHK(sample->SetSampleTime(time));
    CHK(sample->AddBuffer(buffer.Get()));

    return sample;
}