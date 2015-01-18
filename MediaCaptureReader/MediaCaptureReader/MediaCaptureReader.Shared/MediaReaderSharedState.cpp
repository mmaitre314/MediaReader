#include "pch.h"
#include "MediaReaderReadResult.h"
#include "ReaderSharedState.h"
#include "MediaReaderSharedState.h"
#include "MediaReaderStreams.h"
#include "SourceReaderCallback.h"
#include "MediaReader.h"
#include "MediaGraphicsDevice.h"
#include "MediaSample.h"

using namespace std;
using namespace Microsoft::WRL;
using namespace concurrency;
using namespace MediaCaptureReader;
using namespace Platform;
using namespace Platform::Collections;
using namespace Windows::Foundation;
using namespace Windows::Storage::Streams;
using namespace Windows::Media::Core;
using namespace Windows::Media::MediaProperties;
using namespace Windows::Foundation::Collections;

MediaReaderSharedState::MediaReaderSharedState(
    const ComPtr<IUnknown>& source, 
    MediaReaderInitializationSettings^ settings
    )
: _streamCount(0)
, _closed(false)
{
    TraceScopeCx(this);

    _graphicsDevice = _GetSharedGraphicsDevice(settings);

    ComPtr<SourceReaderCallback> callback;
    CHK(MakeAndInitialize<SourceReaderCallback>(&callback, WeakReference(this)));

    auto attr = _CreateSourceReaderAttributes(settings, _graphicsDevice, callback);

    ComPtr<IMFReadWriteClassFactory> readerFactory;
    ComPtr<IMFSourceReaderEx> sourceReader;
    CHK(CoCreateInstance(CLSID_MFReadWriteClassFactory, nullptr, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&readerFactory)));
    CHK(readerFactory->CreateInstanceFromObject(CLSID_MFSourceReader, source.Get(), attr.Get(), IID_PPV_ARGS(&_sourceReader)));
}

MediaReaderSharedState::~MediaReaderSharedState()
{
    TraceScopeCx(this);
}

ComPtr<IMFAttributes> MediaReaderSharedState::_CreateSourceReaderAttributes(
    MediaReaderInitializationSettings^ settings, 
    MediaGraphicsDevice^ graphicsDevice,
    const ComPtr<SourceReaderCallback>& callback
    ) const
{
    TraceScopeCx(const_cast<MediaReaderSharedState^>(this));

    ComPtr<IMFAttributes> attr;
    CHK(MFCreateAttributes(&attr, 10));

    CHK(attr->SetUnknown(MF_SOURCE_READER_ASYNC_CALLBACK, static_cast<IMFSourceReaderCallback*>(callback.Get())));
    CHK(attr->SetUINT32(MF_SOURCE_READER_ENABLE_TRANSCODE_ONLY_TRANSFORMS, true));
    if (settings->ConvertersEnabled)
    {
        CHK(attr->SetUINT32(MF_SOURCE_READER_ENABLE_ADVANCED_VIDEO_PROCESSING, true));
    }
    else
    {
        CHK(attr->SetUINT32(MF_READWRITE_DISABLE_CONVERTERS, true));
    }
    if (graphicsDevice != nullptr)
    {
        CHK(attr->SetUnknown(MF_SOURCE_READER_D3D_MANAGER, graphicsDevice->GetDeviceManager().Get()));
        CHK(attr->SetUINT32(MF_READWRITE_ENABLE_HARDWARE_TRANSFORMS, true));
    }
    else
    {
        CHK(attr->SetUINT32(MF_SOURCE_READER_DISABLE_DXVA, true));
    }
    if (settings->LowLatencyEnabled)
    {
        CHK(attr->SetUINT32(MF_LOW_LATENCY, true));
    }

    return attr;
}

void MediaReaderSharedState::CreateStreams(
    _Outptr_ MediaReaderAudioStream^* audioStream,
    _Outptr_ MediaReaderVideoStream^* videoStream,
    _Outptr_ IVectorView<IMediaReaderStream^>^* allStreams
    )
{
    auto lock = _lock.LockExclusive();
    TraceScopeCx(this);

    MediaReaderAudioStream^ audioStreamTemp;
    MediaReaderVideoStream^ videoStreamTemp;
    auto allStreamsTemp = ref new Vector<IMediaReaderStream^>();

    ComPtr<IMFMediaType> currentMediaType;
    unsigned int streamIndex = 0;
    while (SUCCEEDED(_sourceReader->GetCurrentMediaType(streamIndex, &currentMediaType)))
    {
        GUID type = GUID_NULL;
        (void)currentMediaType->GetMajorType(&type);
        if (type == MFMediaType_Audio)
        {
            auto stream = ref new MediaReaderAudioStream(streamIndex, this);
            allStreamsTemp->Append(stream);
            if (audioStreamTemp == nullptr)
            {
                audioStreamTemp = stream;
            }
        }
        else if (type == MFMediaType_Video)
        {
            auto stream = ref new MediaReaderVideoStream(streamIndex, this);
            allStreamsTemp->Append(stream);
            if (videoStreamTemp == nullptr)
            {
                videoStreamTemp = stream;
            }
        }
        else if (type == MFMediaType_Image)
        {
            auto stream = ref new MediaReaderOtherStream(streamIndex, this);
            allStreamsTemp->Append(stream);
        }
        else
        {
            // ignore (would need other xxxEncodingProperties to support MFMediaType_Stream, etc.)
        }

        streamIndex++;
    }

    _streamCount = streamIndex;

    _sampleRequestQueues.resize(_streamCount);

    *audioStream = audioStreamTemp;
    *videoStream = videoStreamTemp;
    *allStreams = allStreamsTemp->GetView();
}

void MediaReaderSharedState::GetMetadata(WF::TimeSpan* duration, bool* canSeek)
{
    auto lock = _lock.LockExclusive();
    TraceScopeCx(this);

    duration->Duration = 0;
    *canSeek = false;

    PROPVARIANT var;
    HRESULT hr = _sourceReader->GetPresentationAttribute(MF_SOURCE_READER_MEDIASOURCE, MF_PD_DURATION, &var);
    if (SUCCEEDED(hr))
    {
        if (VT_UI8 == var.vt)
        {
            duration->Duration = var.uhVal.QuadPart;
        }
        PropVariantClear(&var);
    }

    CHK(_sourceReader->GetPresentationAttribute(MF_SOURCE_READER_MEDIASOURCE, MF_SOURCE_READER_MEDIASOURCE_CHARACTERISTICS, &var));
    *canSeek = !!(MFMEDIASOURCE_CAN_SEEK & var.ulVal);
    PropVariantClear(&var);
}

AudioEncodingProperties^ MediaReaderSharedState::GetCurrentAudioStreamProperties(unsigned int streamIndex)
{
    auto lock = _lock.LockExclusive();
    TraceScopeCx(this);
    _VerifyNotClosed();

    Microsoft::WRL::ComPtr<IMFMediaType> mt;
    CHK(_sourceReader->GetCurrentMediaType(streamIndex, &mt));

    Microsoft::WRL::ComPtr<ABI::Windows::Media::MediaProperties::IAudioEncodingProperties> props;
    CHK(MFCreatePropertiesFromMediaType(mt.Get(), IID_PPV_ARGS(&props)));

    return reinterpret_cast<AudioEncodingProperties^>(props.Get());
}

VideoEncodingProperties^ MediaReaderSharedState::GetCurrentVideoStreamProperties(unsigned int streamIndex)
{
    auto lock = _lock.LockExclusive();
    TraceScopeCx(this);
    _VerifyNotClosed();

    Microsoft::WRL::ComPtr<IMFMediaType> mt;
    CHK(_sourceReader->GetCurrentMediaType(streamIndex, &mt));

    Microsoft::WRL::ComPtr<ABI::Windows::Media::MediaProperties::IVideoEncodingProperties> props;
    CHK(MFCreatePropertiesFromMediaType(mt.Get(), IID_PPV_ARGS(&props)));

    return reinterpret_cast<VideoEncodingProperties^>(props.Get());
}

IMediaEncodingProperties^ MediaReaderSharedState::GetCurrentOtherStreamProperties(unsigned int streamIndex)
{
    auto lock = _lock.LockExclusive();
    TraceScopeCx(this);
    _VerifyNotClosed();

    Microsoft::WRL::ComPtr<IMFMediaType> mt;
    CHK(_sourceReader->GetCurrentMediaType(streamIndex, &mt));

    Microsoft::WRL::ComPtr<ABI::Windows::Media::MediaProperties::IMediaEncodingProperties> props;
    CHK(MFCreatePropertiesFromMediaType(mt.Get(), IID_PPV_ARGS(&props)));

    return reinterpret_cast<IMediaEncodingProperties^>(props.Get());
}

IVectorView<AudioEncodingProperties^>^ MediaReaderSharedState::GetNativeAudioStreamProperties(unsigned int streamIndex)
{
    auto lock = _lock.LockExclusive();
    TraceScopeCx(this);
    _VerifyNotClosed();

    auto list = ref new Vector<AudioEncodingProperties^>();

    ComPtr<IMFMediaType> mt;
    unsigned int mtIndex = 0;
    while (SUCCEEDED(_sourceReader->GetNativeMediaType(streamIndex, mtIndex, &mt)))
    {
        ComPtr<ABI::Windows::Media::MediaProperties::IAudioEncodingProperties> props;
        if (SUCCEEDED(MFCreatePropertiesFromMediaType(mt.Get(), IID_PPV_ARGS(&props))))
        {
            list->Append(reinterpret_cast<AudioEncodingProperties^>(props.Get()));
        }
        mtIndex++;
    }

    return list->GetView();
}

IVectorView<VideoEncodingProperties^>^ MediaReaderSharedState::GetNativeVideoStreamProperties(unsigned int streamIndex)
{
    auto lock = _lock.LockExclusive();
    TraceScopeCx(this);
    _VerifyNotClosed();

    auto list = ref new Vector<VideoEncodingProperties^>();

    ComPtr<IMFMediaType> mt;
    unsigned int mtIndex = 0;
    while (SUCCEEDED(_sourceReader->GetNativeMediaType(streamIndex, mtIndex, &mt)))
    {
        ComPtr<ABI::Windows::Media::MediaProperties::IVideoEncodingProperties> props;
        if (SUCCEEDED(MFCreatePropertiesFromMediaType(mt.Get(), IID_PPV_ARGS(&props))))
        {
            list->Append(reinterpret_cast<VideoEncodingProperties^>(props.Get()));
        }
        mtIndex++;
    }

    return list->GetView();
}

IVectorView<IMediaEncodingProperties^>^ MediaReaderSharedState::GetNativeOtherStreamProperties(unsigned int streamIndex)
{
    auto lock = _lock.LockExclusive();
    TraceScopeCx(this);
    _VerifyNotClosed();

    auto list = ref new Vector<IMediaEncodingProperties^>();

    ComPtr<IMFMediaType> mt;
    unsigned int mtIndex = 0;
    while (SUCCEEDED(_sourceReader->GetNativeMediaType(streamIndex, mtIndex, &mt)))
    {
        ComPtr<ABI::Windows::Media::MediaProperties::IMediaEncodingProperties> props;
        if (SUCCEEDED(MFCreatePropertiesFromMediaType(mt.Get(), IID_PPV_ARGS(&props))))
        {
            list->Append(reinterpret_cast<IMediaEncodingProperties^>(props.Get()));
        }
        mtIndex++;
    }

    return list->GetView();
}

IAsyncOperation<MediaReaderReadResult^>^ MediaReaderSharedState::ReadAsync(unsigned int streamIndex)
{
    TraceScopeCx(this);

    return create_async([this, streamIndex]()
    {
        return create_task([this, streamIndex]()
        {
            auto lock = _lock.LockExclusive();
            _VerifyNotClosed();

            CHK(_sourceReader->ReadSample(
                streamIndex,
                0,
                nullptr,
                nullptr,
                nullptr,
                nullptr
                ));

            task_completion_event<MediaReaderReadResult^> taskEvent;
            _sampleRequestQueues[streamIndex].push(taskEvent);

            return create_task(taskEvent);
        });
    });
}

IAsyncAction^ MediaReaderSharedState::SetCurrentStreamPropertiesAsync(unsigned int streamIndex, IMediaEncodingProperties^ properties)
{
    TraceScopeCx(this);

    return create_async([this, streamIndex, properties]()
    {
        return create_task([this, streamIndex, properties]()
        {
            auto lock = _lock.LockExclusive();
            _VerifyNotClosed();

            ComPtr<IMFMediaType> mt;
            CHK(MFCreateMediaTypeFromProperties(reinterpret_cast<IUnknown*>(properties), &mt));

            CHK(_sourceReader->SetCurrentMediaType(streamIndex, nullptr, mt.Get()));
        });
    });
}

IAsyncAction^ MediaReaderSharedState::SetNativeStreamPropertiesAsync(unsigned int streamIndex, IMediaEncodingProperties^ properties)
{
    TraceScopeCx(this);

    return create_async([this, streamIndex, properties]()
    {
        return create_task([this, streamIndex, properties]()
        {
            auto lock = _lock.LockExclusive();
            _VerifyNotClosed();

            ComPtr<IMFMediaType> mt;
            CHK(MFCreateMediaTypeFromProperties(reinterpret_cast<IUnknown*>(properties), &mt));

            DWORD flags;
            CHK(_sourceReader->SetNativeMediaType(streamIndex, mt.Get(), &flags));

            if (flags & MF_SOURCE_READERF_ALLEFFECTSREMOVED)
            {
                // ignored for now
            }
            else if (flags & MF_SOURCE_READERF_CURRENTMEDIATYPECHANGED)
            {
                // ignored for now
            }
        });
    });
}

void MediaReaderSharedState::SetSelection(unsigned int streamIndex, bool selected)
{
    auto lock = _lock.LockExclusive();
    TraceScopeCx(this);
    _VerifyNotClosed();

    CHK(_sourceReader->SetStreamSelection(streamIndex, selected));
}

bool MediaReaderSharedState::GetSelection(unsigned int streamIndex)
{
    auto lock = _lock.LockExclusive();
    TraceScopeCx(this);

    if (!_closed)
    {
        BOOL selected;
        CHK(_sourceReader->GetStreamSelection(streamIndex, &selected));
        return !!selected;
    }
    else
    {
        return false;
    }
}

void MediaReaderSharedState::OnReadSample(
    HRESULT hr,
    DWORD streamIndex,
    MF_SOURCE_READER_FLAG streamFlags,
    long long time,
    _In_opt_ IMFSample *sample
    )
{
    auto lock = _lock.LockExclusive();
    TraceScopeCx(this);

    try
    {
        IMediaSample^ mediaSample;

        if (sample != nullptr)
        {
            long long duration = 0;
            (void)sample->GetSampleDuration(&duration);

#ifndef NDEBUG
            long long timeSample = 0;
            (void)sample->GetSampleTime(&timeSample);
            NT_ASSERT(timeSample == time);
#endif

            Trace("@%p IMFSample @%p, time %I64dhns, duration %I64dhns", (void*)this, sample, (int64)time, (int64)duration);

            ComPtr<IMF2DBuffer2> buffer2D;
            ComPtr<IMFMediaBuffer> buffer1D;
            CHK(sample->GetBufferByIndex(0, &buffer1D));
            if (SUCCEEDED(buffer1D.As(&buffer2D)))
            {
                ComPtr<IMFMediaType> mt;
                CHK(_sourceReader->GetCurrentMediaType(streamIndex, &mt));

                GUID subtype;
                CHK(mt->GetGUID(MF_MT_SUBTYPE, &subtype));

                unsigned int width;
                unsigned int height;
                CHK(MFGetAttributeSize(mt.Get(), MF_MT_FRAME_SIZE, &width, &height));

                auto mediaSample2D = ref new MediaSample2D(
                    sample,
                    MediaSample2D::GetFormatFromSubType(subtype),
                    (int)width,
                    (int)height,
                    _graphicsDevice
                    );
                mediaSample = mediaSample2D;
            }
            else
            {
                mediaSample = ref new MediaSample1D(sample);
            }

            mediaSample->Duration = TimeSpan{ duration };
            mediaSample->Timestamp = TimeSpan{ time };
        }
        else
        {
            Trace("@%p IMFSample @nullptr, time %I64dhns", (void*)this, time);
        }

        auto result = ref new MediaReaderReadResult(hr, streamFlags, time, mediaSample);

        _sampleRequestQueues[streamIndex].front().set(result);
    }
    catch (Exception^ e)
    {
        _sampleRequestQueues[streamIndex].front().set_exception(e);
    }
    catch (std::exception e)
    {
        _sampleRequestQueues[streamIndex].front().set_exception(e);
    }

    _sampleRequestQueues[streamIndex].pop();
}

MediaGraphicsDevice^ MediaReaderSharedState::_GetSharedGraphicsDevice(MediaReaderInitializationSettings^ settings) const
{
    TraceScopeCx(const_cast<MediaReaderSharedState^>(this));
    MediaGraphicsDevice^ graphicsDevice;

    if (settings->HardwareAccelerationEnabled)
    {
        graphicsDevice = settings->GraphicsDevice;
        if (graphicsDevice == nullptr)
        {
            graphicsDevice = ref new MediaGraphicsDevice();
        }
    }

    return graphicsDevice;
}

void MediaReaderSharedState::Seek(WF::TimeSpan position)
{
    auto lock = _lock.LockExclusive();
    TraceScopeCx(this);
    _VerifyNotClosed();

    PROPVARIANT seek;
    seek.vt = VT_I8;
    seek.hVal.QuadPart = position.Duration;

    CHK(_sourceReader->SetCurrentPosition(GUID_NULL, seek));
}

IAsyncAction^ MediaReaderSharedState::FinishAsync()
{
    auto lock = _lock.LockExclusive();
    TraceScopeCx(this);

    if (!_closed)
    {
        _closed = true;

        _sourceReader = nullptr;
        _graphicsDevice = nullptr;
        _streamCount = 0;
    }

    return create_async([]()
    {
        return task_from_result();
    });
}

