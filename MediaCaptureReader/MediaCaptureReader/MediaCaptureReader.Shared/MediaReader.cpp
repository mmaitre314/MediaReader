#include "pch.h"
#include "MediaReaderReadResult.h"
#include "ReaderSharedState.h"
#include "MediaReaderSharedState.h"
#include "CaptureReaderSharedState.h"
#include "MediaReaderStreams.h"
#include "MediaReader.h"
#include "SourceReaderCallback.h"

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
using namespace Windows::Storage;
using namespace Windows::Media::Capture;

IAsyncOperation<MediaReader^>^ MediaReader::CreateFromPathAsync(
    String^ absolutePath,
    AudioInitialization audio,
    VideoInitialization video,
    MediaReaderInitializationSettings^ settings
    )
{
    return create_async([absolutePath, audio, video, settings]()
    {
        return create_task(StorageFile::GetFileFromApplicationUriAsync(ref new WF::Uri(absolutePath))).then([audio, video, settings](StorageFile^ file)
        {
            return MediaReader::CreateFromFileAsync(file, audio, video, settings);
        });
    });
}

IAsyncOperation<MediaReader^>^ MediaReader::CreateFromFileAsync(
    IStorageFile^ file,
    AudioInitialization audio,
    VideoInitialization video,
    MediaReaderInitializationSettings^ settings
    )
{
    return create_async([file, audio, video, settings]()
    {
        return create_task(file->OpenReadAsync()).then([audio, video, settings](IRandomAccessStreamWithContentType^ stream)
        {
            return MediaReader::CreateFromStreamAsync(stream, audio, video, settings);
        });
    });
}

IAsyncOperation<MediaReader^>^ MediaReader::CreateFromStreamAsync(
    IRandomAccessStream^ stream, 
    AudioInitialization audio,
    VideoInitialization video,
    MediaReaderInitializationSettings^ settings
    )
{
    return create_async([stream, audio, video, settings]()
    {
        return create_task([stream, audio, video, settings]()
        {
            auto reader = ref new MediaReader(stream, settings);

            return create_task(reader->_InitializeAsync(audio, video)).then([reader]()
            {
                return reader;
            });
        });
    });
}

IAsyncOperation<MediaReader^>^ MediaReader::CreateFromMediaSourceAsync(
    IMediaSource^ source, 
    AudioInitialization audio,
    VideoInitialization video,
    MediaReaderInitializationSettings^ settings
    )
{
    return create_async([source, audio, video, settings]()
    {
        return create_task([source, audio, video, settings]()
        {
            auto reader = ref new MediaReader(source, settings);

            return create_task(reader->_InitializeAsync(audio, video)).then([reader]()
            {
                return reader;
            });
        });
    });
}

IAsyncOperation<MediaReader^>^ MediaReader::CreateFromMediaCaptureAsync(
    MediaCapture^ source,
    AudioInitialization audio,
    VideoInitialization video,
    MediaReaderCaptureInitializationSettings^ settings
    )
{
    Agile<MediaCapture> capture(source);

    return create_async([capture, audio, video, settings]()
    {
        return create_task([capture, audio, video, settings]()
        {
            auto reader = ref new MediaReader(capture.Get(), settings);

            return create_task(reader->_InitializeAsync(audio, video)).then([reader]()
            {
                return reader;
            });
        });
    });
}


MediaReader::MediaReader(IRandomAccessStream^ stream, MediaReaderInitializationSettings^ settings)
: _canSeek(false)
{
    TraceScopeCx(this);

    ComPtr<IMFByteStream> streamNative;
    CHK(MFCreateMFByteStreamOnStreamEx(reinterpret_cast<IUnknown*>(stream), &streamNative));

    auto state = ref new MediaReaderSharedState(streamNative, settings);

    state->CreateStreams(&_audioStream, &_videoStream, &_allStreams);
    state->GetMetadata(&_duration, &_canSeek);

    _state = static_cast<IReaderSharedState^>(state);
}

MediaReader::MediaReader(IMediaSource^ source, MediaReaderInitializationSettings^ settings)
: _canSeek(false)
{
    TraceScopeCx(this);

    ComPtr<IMFGetService> mfService;
    ComPtr<IMFMediaSource> sourceNative;
    CHK(reinterpret_cast<IUnknown*>(source)->QueryInterface(IID_PPV_ARGS(&mfService)));
    CHK(mfService->GetService(MF_MEDIASOURCE_SERVICE, IID_PPV_ARGS(&sourceNative)));

    auto state = ref new MediaReaderSharedState(sourceNative, settings);

    state->CreateStreams(&_audioStream, &_videoStream, &_allStreams);
    state->GetMetadata(&_duration, &_canSeek);

    _state = static_cast<IReaderSharedState^>(state);
}

MediaReader::MediaReader(MediaCapture^ capture, MediaReaderCaptureInitializationSettings^ settings)
: _canSeek(false)
{
    TraceScopeCx(this);

    auto state = ref new CaptureReaderSharedState(capture, settings);

    state->CreateStreams(&_audioStream, &_videoStream, &_allStreams);

    _state = static_cast<IReaderSharedState^>(state);
}

MediaReader::~MediaReader()
{
    auto lock = _lock.LockExclusive();
    TraceScopeCx(this);

    for (auto stream : _allStreams)
    {
        auto audioStream = dynamic_cast<MediaReaderAudioStream^>(stream);
        auto videoStream = dynamic_cast<MediaReaderVideoStream^>(stream);
        auto otherStream = dynamic_cast<MediaReaderOtherStream^>(stream);

        if (audioStream != nullptr)
        {
            audioStream->Close();
        }
        if (videoStream != nullptr)
        {
            videoStream->Close();
        }
        if (otherStream != nullptr)
        {
            otherStream->Close();
        }
    }

    if (_state != nullptr)
    {
        (void) _state->FinishAsync();
        _state = nullptr;
    }
}

IAsyncAction^ MediaReader::_InitializeAsync(
    AudioInitialization audio,
    VideoInitialization video
    )
{
    TraceScopeCx(this);

    // Deselect all the streams
    for (auto stream : _allStreams)
    {
        stream->SetSelection(false);
    }

    return create_async([this, audio, video]()
    {
        // Update the audio output
        task<void> taskAudio;
        if (_audioStream == nullptr)
        {
            taskAudio = task_from_result();
        }
        else
        {
            switch (audio)
            {
            case AudioInitialization::Pcm:
                {
                    _audioStream->SetSelection(true);
                    auto props = _audioStream->GetCurrentStreamProperties();
                    auto newProps = AudioEncodingProperties::CreatePcm(props->SampleRate, props->ChannelCount, 16);
                    taskAudio = create_task(_audioStream->SetCurrentStreamPropertiesAsync(newProps));
                }
                break;

            case AudioInitialization::PassThrough:
                _audioStream->SetSelection(true);
                taskAudio = task_from_result();
                break;

            case AudioInitialization::Deselected: 
                taskAudio = task_from_result();
                break;

            default: NT_ASSERT(false); throw ref new COMException(E_UNEXPECTED);
            }
        }

        return taskAudio.then([this, video]()
        {
            // Update the video output
            task<void> taskVideo;
            if (_videoStream == nullptr)
            {
                taskVideo = task_from_result();
            }
            else
            {
                switch (video)
                {
                case VideoInitialization::Nv12:
                    {
                        _videoStream->SetSelection(true);
                        auto props = _videoStream->GetCurrentStreamProperties();
                        auto newProps = VideoEncodingProperties::CreateUncompressed(MediaEncodingSubtypes::Nv12, props->Width, props->Height);
                        newProps->FrameRate->Numerator = props->FrameRate->Numerator;
                        newProps->FrameRate->Denominator = props->FrameRate->Denominator;
                        taskVideo = create_task(_videoStream->SetCurrentStreamPropertiesAsync(newProps));
                    }
                    break;

                case VideoInitialization::Bgra8:
                    {
                        _videoStream->SetSelection(true);
                        auto props = _videoStream->GetCurrentStreamProperties();
                        auto newProps = VideoEncodingProperties::CreateUncompressed(MediaEncodingSubtypes::Bgra8, props->Width, props->Height);
                        newProps->FrameRate->Numerator = props->FrameRate->Numerator;
                        newProps->FrameRate->Denominator = props->FrameRate->Denominator;
                        taskVideo = create_task(_videoStream->SetCurrentStreamPropertiesAsync(newProps));
                    }
                    break;

                case VideoInitialization::PassThrough:
                    _videoStream->SetSelection(true);
                    taskVideo = task_from_result();
                    break;

                case VideoInitialization::Deselected:
                    taskVideo = task_from_result();
                    break;

                default: NT_ASSERT(false); throw ref new COMException(E_UNEXPECTED);
                }
            }

            return taskVideo;
        }).then([this]()
        {
            return _state->CompleteInitializationAsync();
        });
    });
}

void MediaReader::Seek(TimeSpan position)
{
    auto lock = _lock.LockExclusive();
    TraceScopeCx(this);
    _VerifyNotClosed();

    _state->Seek(position);
}

// TODO
//Object^ MediaReader::GetService(Guid service)
//{
//    auto lock = _lock.LockExclusive();
//    TraceScopeCx(this);
//    _VerifyNotClosed();
//
//    if (service.Equals(__uuidof(IMediaReaderNative)))
//    {
//        auto state = dynamic_cast<MediaReaderSharedState^>(_state);
//        if (state != nullptr)
//        {
//            ComPtr<MediaReaderNative> wrapper;
//            CHK(MakeAndInitialize<MediaReaderNative>(&wrapper, state->SourceReader()));
//
//            return reinterpret_cast<Object^>(static_cast<IMediaReaderNative*>(wrapper.Get()));
//        }
//    }
//
//    return nullptr;
//}
