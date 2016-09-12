#include "pch.h"
#include "ReaderSharedState.h"
#include "MediaStreamSink.h"
#include "MediaSink.h"
#include "MediaReaderReadResult.h"
#include "CaptureReaderSharedState.h"
#include "MediaReaderStreams.h"
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
using namespace Windows::Media;
using namespace Windows::Media::Core;
using namespace Windows::Media::Capture;
using namespace Windows::Media::MediaProperties;
using namespace Windows::Foundation::Collections;

CaptureReaderSharedState::CaptureReaderSharedState(MediaCapture^ capture, MediaReaderCaptureInitializationSettings^ settings)
: _state(State::Created)
, _streamType(settings->Stream)
, _streamCount(0)
, _audioSelected(false)
, _videoSelected(false)
{
	#ifdef NTRACELOG
		TraceScopeCx(this);
		Trace("@%p StreamType %i", (void*)this, (int)_streamType);
	#endif
	
    _capture = capture;
    _graphicsDevice = MediaGraphicsDevice::CreateFromMediaCapture(capture);
    
    auto mode = capture->MediaCaptureSettings->StreamingCaptureMode;

    _audioSelected = (mode == StreamingCaptureMode::Audio) || (mode == StreamingCaptureMode::AudioAndVideo);

    auto audioProps =
        _audioSelected ?
        safe_cast<AudioEncodingProperties^>(capture->AudioDeviceController->GetMediaStreamProperties(MediaStreamType::Audio)) :
        nullptr;

    _videoSelected = (mode == StreamingCaptureMode::Video) || (mode == StreamingCaptureMode::AudioAndVideo);

    auto videoProps =
        _videoSelected ?
        safe_cast<VideoEncodingProperties^>(capture->VideoDeviceController->GetMediaStreamProperties(_GetVideoStreamType())) :
        nullptr;

    _encodingProfile = ref new MediaEncodingProfile();
    _encodingProfile->Audio = audioProps;
    _encodingProfile->Video = videoProps;

    _mediaSink = Make<MediaSink>(
        audioProps,
        videoProps,
        ref new MediaSampleHandler(this, &CaptureReaderSharedState::ProcessAudioSample),
        ref new MediaSampleHandler(this, &CaptureReaderSharedState::ProcessVideoSample),
        _graphicsDevice
        );
    CHKOOM(_mediaSink);

    _mediaExtension = reinterpret_cast<IMediaExtension^>(static_cast<AWM::IMediaExtension*>(_mediaSink.Get()));
}

CaptureReaderSharedState::~CaptureReaderSharedState()
{
	#ifdef NTRACELOG
		TraceScopeCx(this);
	#endif
}

IAsyncAction^ CaptureReaderSharedState::CompleteInitializationAsync()
{
    return create_async([this]()
    {
        auto lock = _lock.LockExclusive();

		#ifdef NTRACELOG
			TraceScopeCx(this);
		#endif
        
        _state = State::Starting;

        task<void> task;
        if (_streamType == CaptureStreamType::Preview)
        {
            task = create_task(_capture->StartPreviewToCustomSinkAsync(_encodingProfile, _mediaExtension));
        }
        else
        {
            task = create_task(_capture->StartRecordToCustomSinkAsync(_encodingProfile, _mediaExtension));
        }

        return task.then([this]()
        {
            _state = State::Started;
        });
    });
}

IAsyncAction^ CaptureReaderSharedState::FinishAsync()
{
    auto lock = _lock.LockExclusive();

	#ifdef NTRACELOG
		TraceScopeCx(this);
	#endif

    auto streamType = _streamType;
    bool stopNeeded = (_state == State::Started);

    if ((_state == State::Closing) || (_state == State::Closed))
    {
        return create_async([]()
        {
            return task_from_result();
        });
    }
    _state = State::Closed;

    ComPtr<MediaSink> mediaSink;
    Agile<MediaCapture> capture;
    if (stopNeeded)
    {
        capture = _capture;
        mediaSink = _mediaSink;
    }

    _mediaExtension = nullptr;
    _mediaSink = nullptr;
    _graphicsDevice = nullptr;
    _capture = nullptr;
    _streamCount = 0;
    _encodingProfile = nullptr;

    if (stopNeeded)
    {
        // Need to wait for preview/record to be stopped before shutting down the sink
        return create_async([capture, mediaSink, stopNeeded, streamType]()
        {
            task<void> task;
            if (streamType == CaptureStreamType::Preview)
            {
                task = create_task(capture->StopPreviewAsync());
            }
            else
            {
                task = create_task(capture->StopRecordAsync());
            }

            return task.then([mediaSink]()
            {
                mediaSink->Shutdown();
            });
        });
    }
    else
    {
        if (mediaSink != nullptr)
        {
            mediaSink->Shutdown();
        }

        return create_async([]()
        {
            return task_from_result();
        });
    }
}

void CaptureReaderSharedState::CreateStreams(
    _Outptr_ MediaReaderAudioStream^* audioStream,
    _Outptr_ MediaReaderVideoStream^* videoStream,
    _Outptr_ IVectorView<IMediaReaderStream^>^* allStreams
    )
{
    auto lock = _lock.LockExclusive();

	#ifdef NTRACELOG
		TraceScopeCx(this);
	#endif

    MediaReaderAudioStream^ audioStreamTemp;
    MediaReaderVideoStream^ videoStreamTemp;
    auto allStreamsTemp = ref new Vector<IMediaReaderStream^>();

    switch (_capture->MediaCaptureSettings->StreamingCaptureMode)
    {
    case StreamingCaptureMode::Audio:
        audioStreamTemp = ref new MediaReaderAudioStream(0, this);
        _streamCount = 1;
        break;

    case StreamingCaptureMode::Video:
        videoStreamTemp = ref new MediaReaderVideoStream(0, this);
        _streamCount = 1;
        break;

    case StreamingCaptureMode::AudioAndVideo:
        audioStreamTemp = ref new MediaReaderAudioStream(0, this);
        videoStreamTemp = ref new MediaReaderVideoStream(1, this);
        _streamCount = 2;
        break;

    default:
        NT_ASSERT(false); throw ref new COMException(E_UNEXPECTED);
    }

    *audioStream = audioStreamTemp;
    *videoStream = videoStreamTemp;
    *allStreams = allStreamsTemp->GetView();
}

IAsyncOperation<MediaReaderReadResult^>^ CaptureReaderSharedState::ReadAudioAsync(unsigned int /*streamIndex*/)
{
    return create_async([this]()
    {
        auto lock = _lock.LockExclusive();

		#ifdef NTRACELOG
			TraceScopeCx(this);
		#endif
        
        if (_state != State::Started)
        {
            throw ref new Platform::COMException(MF_E_INVALID_STREAM_STATE, L"Not started");
        }

        _mediaSink->RequestAudioSample();

        task_completion_event<MediaReaderReadResult^> taskEvent;
        _audioSampleRequestQueue.push(taskEvent);

        return create_task(taskEvent);
    });
}

IAsyncOperation<MediaReaderReadResult^>^ CaptureReaderSharedState::ReadVideoAsync(unsigned int /*streamIndex*/)
{
    return create_async([this]()
    {
        auto lock = _lock.LockExclusive();

		#ifdef NTRACELOG
			TraceScopeCx(this);
		#endif

        if (_state != State::Started)
        {
            throw ref new Platform::COMException(MF_E_INVALID_STREAM_STATE, L"Not started");
        }

        _mediaSink->RequestVideoSample();

        task_completion_event<MediaReaderReadResult^> taskEvent;
        _videoSampleRequestQueue.push(taskEvent);

        return create_task(taskEvent);
    });
}

IAsyncAction^ CaptureReaderSharedState::SetCurrentAudioStreamPropertiesAsync(unsigned /*streamIndex*/, _In_ AudioEncodingProperties^ properties)
{
    auto lock = _lock.LockExclusive();

	#ifdef NTRACELOG
		TraceScopeCx(this);
	#endif
    
    _VerifyNotClosed();

    _encodingProfile->Audio = properties;

    ComPtr<IMFMediaType> mt;
    CHK(MFCreateMediaTypeFromProperties(reinterpret_cast<IUnknown*>(properties), &mt));

    _mediaSink->SetCurrentAudioMediaType(mt.Get());

    if ((_state == State::Starting) || (_state == State::Started))
    {
        return _capture->SetEncodingPropertiesAsync(MediaStreamType::Audio, properties, nullptr);
    }
    else
    {
        return create_async([]()
        {
            return task_from_result();
        });
    }
}

IAsyncAction^ CaptureReaderSharedState::SetCurrentVideoStreamPropertiesAsync(unsigned /*streamIndex*/, _In_ VideoEncodingProperties^ properties)
{
    auto lock = _lock.LockExclusive();

	#ifdef NTRACELOG
		TraceScopeCx(this);
	#endif

    _VerifyNotClosed();

    _encodingProfile->Video = properties;

    ComPtr<IMFMediaType> mt;
    CHK(MFCreateMediaTypeFromProperties(reinterpret_cast<IUnknown*>(properties), &mt));

    _mediaSink->SetCurrentVideoMediaType(mt.Get());

    if ((_state == State::Starting) || (_state == State::Started))
    {
        return _capture->SetEncodingPropertiesAsync(_GetVideoStreamType(), properties, nullptr);
    }
    else
    {
        return create_async([]()
        {
            return task_from_result();
        });
    }
}

IAsyncAction^ CaptureReaderSharedState::SetNativeAudioStreamPropertiesAsync(unsigned int /*streamIndex*/, _In_ AudioEncodingProperties^ properties)
{
    auto lock = _lock.LockExclusive();
	#ifdef NTRACELOG
		TraceScopeCx(this);
	#endif    
    _VerifyNotClosed();

    return _capture->AudioDeviceController->SetMediaStreamPropertiesAsync(MediaStreamType::Audio, properties);
}

IAsyncAction^ CaptureReaderSharedState::SetNativeVideoStreamPropertiesAsync(unsigned int /*streamIndex*/, _In_ VideoEncodingProperties^ properties)
{
    auto lock = _lock.LockExclusive();
	#ifdef NTRACELOG
		TraceScopeCx(this);
	#endif   
    _VerifyNotClosed();

    return _capture->VideoDeviceController->SetMediaStreamPropertiesAsync(_GetVideoStreamType(), properties);
}

AudioEncodingProperties^ CaptureReaderSharedState::GetCurrentAudioStreamProperties(unsigned int /*streamIndex*/)
{
    auto lock = _lock.LockExclusive();
	#ifdef NTRACELOG
		TraceScopeCx(this);
	#endif
    _VerifyNotClosed();

    ComPtr<IMFMediaType> mt;
    ComPtr<IMFMediaTypeHandler> handler;
    ComPtr<IMFStreamSink> stream;
    CHK(_mediaSink->GetStreamSinkById(c_audioStreamSinkId, &stream));
    CHK(stream->GetMediaTypeHandler(&handler));
    CHK(handler->GetCurrentMediaType(&mt));

    ComPtr<ABI::Windows::Media::MediaProperties::IAudioEncodingProperties> props;
    CHK(MFCreatePropertiesFromMediaType(mt.Get(), IID_PPV_ARGS(&props)));

    return reinterpret_cast<AudioEncodingProperties^>(props.Get());
}

VideoEncodingProperties^ CaptureReaderSharedState::GetCurrentVideoStreamProperties(unsigned int /*streamIndex*/)
{
    auto lock = _lock.LockExclusive();
	#ifdef NTRACELOG
		TraceScopeCx(this);
	#endif
    _VerifyNotClosed();

    ComPtr<IMFMediaType> mt;
    ComPtr<IMFMediaTypeHandler> handler;
    ComPtr<IMFStreamSink> stream;
    CHK(_mediaSink->GetStreamSinkById(c_videoStreamSinkId, &stream));
    CHK(stream->GetMediaTypeHandler(&handler));
    CHK(handler->GetCurrentMediaType(&mt));

    ComPtr<ABI::Windows::Media::MediaProperties::IVideoEncodingProperties> props;
    CHK(MFCreatePropertiesFromMediaType(mt.Get(), IID_PPV_ARGS(&props)));

    return reinterpret_cast<VideoEncodingProperties^>(props.Get());
}

IVectorView<AudioEncodingProperties^>^ CaptureReaderSharedState::GetNativeAudioStreamProperties(unsigned int /*streamIndex*/)
{
    auto lock = _lock.LockExclusive();
	#ifdef NTRACELOG
		TraceScopeCx(this);
	#endif  
    _VerifyNotClosed();

    auto availableFormats = _capture->VideoDeviceController->GetAvailableMediaStreamProperties(MediaStreamType::Audio);

    auto nativeFormats = ref new Vector<AudioEncodingProperties^>();

    for (auto format : availableFormats)
    {
        auto nativeFormat = dynamic_cast<AudioEncodingProperties^>(format);
        if (nativeFormat != nullptr)
        {
            nativeFormats->Append(nativeFormat);
        }
        else
        {
			#ifdef NTRACELOG
				Trace("@%p ignoring one available format (not audio)", (void*)this);
			#endif
        }
    }

    return nativeFormats->GetView();
}

IVectorView<VideoEncodingProperties^>^ CaptureReaderSharedState::GetNativeVideoStreamProperties(unsigned int /*streamIndex*/)
{
    auto lock = _lock.LockExclusive();
	#ifdef NTRACELOG
		TraceScopeCx(this);
	#endif
    _VerifyNotClosed();

    auto availableFormats = _capture->VideoDeviceController->GetAvailableMediaStreamProperties(_GetVideoStreamType());

    auto nativeFormats = ref new Vector<VideoEncodingProperties^>();

    for (auto format : availableFormats)
    {
        auto nativeFormat = dynamic_cast<VideoEncodingProperties^>(format);
        if (nativeFormat != nullptr)
        {
            nativeFormats->Append(nativeFormat);
        }
        else
        {
			#ifdef NTRACELOG
				Trace("@%p ignoring one available format (not video)", (void*)this);
			#endif
        }
    }

    return nativeFormats->GetView();
}

void CaptureReaderSharedState::SetAudioSelection(unsigned int /*streamIndex*/, bool selected)
{
    auto lock = _lock.LockExclusive();
	#ifdef NTRACELOG
		TraceScopeCx(this);
	#endif

    if (_state != State::Created)
    {
        throw ref new Platform::COMException(MF_E_INVALID_STREAM_STATE);
    }

    auto mode = _capture->MediaCaptureSettings->StreamingCaptureMode;
    if ((mode != StreamingCaptureMode::Audio) && (mode != StreamingCaptureMode::AudioAndVideo))
    {
        throw ref new COMException(MF_E_STREAMSINK_REMOVED);
    }

    _audioSelected = selected;
}

void CaptureReaderSharedState::SetVideoSelection(unsigned int /*streamIndex*/, bool selected)
{
    auto lock = _lock.LockExclusive();
	#ifdef NTRACELOG
		TraceScopeCx(this);
	#endif

    if (_state != State::Created)
    {
        throw ref new Platform::COMException(MF_E_INVALID_STREAM_STATE);
    }

    auto mode = _capture->MediaCaptureSettings->StreamingCaptureMode;
    if ((mode != StreamingCaptureMode::Video) && (mode != StreamingCaptureMode::AudioAndVideo))
    {
        throw ref new COMException(MF_E_STREAMSINK_REMOVED);
    }

    _videoSelected = selected;
}

bool CaptureReaderSharedState::GetAudioSelection(unsigned int /*streamIndex*/)
{
    auto lock = _lock.LockExclusive();
	#ifdef NTRACELOG
		TraceScopeCx(this);
	#endif
    return _audioSelected;
}

bool CaptureReaderSharedState::GetVideoSelection(unsigned int /*streamIndex*/)
{
    auto lock = _lock.LockExclusive();
	#ifdef NTRACELOG
		TraceScopeCx(this);
	#endif
    return _videoSelected;
}

void CaptureReaderSharedState::ProcessAudioSample(_In_ IMediaSample^ sample)
{
    auto lock = _lock.LockExclusive();
	#ifdef NTRACELOG
		TraceScopeCx(this);
		Trace("@%p IMediaBufferReference @%p", (void*)this, (void*)sample);
	#endif

    try
    {
        auto result = ref new MediaReaderReadResult(S_OK, (MF_SOURCE_READER_FLAG)0, sample->Timestamp.Duration, sample);

        _audioSampleRequestQueue.front().set(result);
    }
    catch (Exception^ e)
    {
        _audioSampleRequestQueue.front().set_exception(e);
    }
    catch (std::exception e)
    {
        _audioSampleRequestQueue.front().set_exception(e);
    }

    _audioSampleRequestQueue.pop();
}

void CaptureReaderSharedState::ProcessVideoSample(_In_ IMediaSample^ sample)
{
    auto lock = _lock.LockExclusive();
	#ifdef NTRACELOG
		TraceScopeCx(this);
		Trace("@%p IMediaBufferReference @%p", (void*)this, (void*)sample);
	#endif

    try
    {
        auto result = ref new MediaReaderReadResult(S_OK, (MF_SOURCE_READER_FLAG)0, sample->Timestamp.Duration, sample);

        _videoSampleRequestQueue.front().set(result);
    }
    catch (Exception^ e)
    {
        _videoSampleRequestQueue.front().set_exception(e);
    }
    catch (std::exception e)
    {
        _videoSampleRequestQueue.front().set_exception(e);
    }

    _videoSampleRequestQueue.pop();
}
