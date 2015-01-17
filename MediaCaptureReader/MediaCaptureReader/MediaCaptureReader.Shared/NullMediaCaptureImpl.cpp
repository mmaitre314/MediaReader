#include "pch.h"
#include "NullMediaCaptureImpl.h"
#include "NullVideoDeviceController.h"
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

    _videoDeviceController = Make<NullVideoDeviceController>();
    CHKOOM(_videoDeviceController);
}

NullMediaCaptureImpl::~NullMediaCaptureImpl()
{
}

STDMETHODIMP NullMediaCaptureImpl::InitializeAsync(_COM_Outptr_ IAsyncAction **asyncInfo)
{
    return ExceptionBoundary([=]()
    {
        auto lock = _lock.LockExclusive();

        CHKNULL(asyncInfo);
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

        CHKNULL(asyncInfo);
        *asyncInfo = nullptr;

        CHKNULL(mediaCaptureInitializationSettings);

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
        CHKNULL(value);
        *value = this;
        (*value)->AddRef();
    });
}

STDMETHODIMP NullMediaCaptureImpl::GetDirectxDeviceManager(_COM_Outptr_ IMFDXGIDeviceManager **value)
{
    return ExceptionBoundary([=]()
    {
        auto lock = _lock.LockExclusive();

        CHKNULL(value);
        CHK(_deviceManager.CopyTo(value));
    });
}

STDMETHODIMP NullMediaCaptureImpl::get_VideoDeviceController(_COM_Outptr_ IVideoDeviceController **value)
{
    return ExceptionBoundary([=]()
    {
        auto lock = _lock.LockExclusive();

        CHKNULL(value);
        CHK(_videoDeviceController.CopyTo(value));
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

        CHKNULL(asyncInfo);
        *asyncInfo = nullptr;

        CHKNULL(customMediaSink);

        if ((_audioStreamSink != nullptr) || (_videoStreamSink != nullptr))
        {
            CHK(OriginateError(E_ILLEGAL_METHOD_CALL))
        }

        auto mediaSink = As<IMFMediaSink>(customMediaSink);

        unsigned long count;
        CHK(mediaSink->GetStreamSinkCount(&count));
        if (count != 1)
        {
            CHK(OriginateError(E_INVALIDARG));
        }

        ComPtr<IMFStreamSink> videoStreamSink;
        ComPtr<IMFMediaTypeHandler> handler;
        ComPtr<IMFMediaType> type;
        CHK(mediaSink->GetStreamSinkByIndex(0, &videoStreamSink));
        CHK(videoStreamSink->GetMediaTypeHandler(&handler));
        CHK(handler->GetCurrentMediaType(&type));

        // Validate media type
        GUID majorType;
        GUID subType;
        CHK(type->GetGUID(MF_MT_MAJOR_TYPE, &majorType));
        CHK(type->GetGUID(MF_MT_SUBTYPE, &subType));
        if (majorType != MFMediaType_Video)
        {
            CHK(OriginateError(E_INVALIDARG));
        }
        if ((subType != MFVideoFormat_ARGB32) && (subType != MFVideoFormat_NV12))
        {
            CHK(OriginateError(E_INVALIDARG));
        }

        _previewFourCC = subType.Data1;
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

        _videoStreamSink = videoStreamSink;

        *asyncInfo = reinterpret_cast<IAsyncAction*>(action);
        (*asyncInfo)->AddRef();
    });
}

STDMETHODIMP NullMediaCaptureImpl::StopPreviewAsync(_COM_Outptr_ IAsyncAction **asyncInfo)
{
    return ExceptionBoundary([=]()
    {
        auto lock = _lock.LockExclusive();

        CHKNULL(asyncInfo);
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
        ComPtr<IMFStreamSink> videoStreamSink;
        MFTIME previewStartTime;
        {
            auto lock = _lock.LockExclusive();
            videoStreamSink = _videoStreamSink;
            if (videoStreamSink == nullptr)
            {
                break;
            }

            previewStartTime = _previewStartTime;
        }

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