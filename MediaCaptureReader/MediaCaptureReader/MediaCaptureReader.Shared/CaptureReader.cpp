#include "pch.h"
#include "MediaSample.h"
#include "MediaStreamSink.h"
#include "MediaSink.h"
#include "CaptureReader.h"

using namespace concurrency;
using namespace MediaCaptureReader;
using namespace Platform;
using namespace Microsoft::WRL;
using namespace Microsoft::WRL::Details;
using namespace Windows::Foundation;
using namespace Windows::Media;
using namespace Windows::Media::Capture;
using namespace Windows::Media::MediaProperties;

IAsyncOperation<CaptureReader^>^ CaptureReader::CreateAsync(
    _In_ MediaCapture^ capture, 
    _In_ MediaEncodingProfile^ profile,
    CaptureStreamType streamType
    )
{
    CHKNULL(capture);
    CHKNULL(profile);

    auto reader = ref new CaptureReader(capture, profile, streamType);

    task<void> task;
    if (reader->_streamType == CaptureStreamType::Preview)
    {
        task = create_task(capture->StartPreviewToCustomSinkAsync(profile, reader->_mediaExtension));
    }
    else
    {
        task = create_task(capture->StartRecordToCustomSinkAsync(profile, reader->_mediaExtension));
    }

    return create_async([task, reader]()
    {
        return task.then([reader]()
        {
            reader->_state = State::Started;
            return reader;
        });
    });
}

CaptureReader::CaptureReader(
    _In_ MediaCapture^ capture,
    _In_ MediaEncodingProfile^ profile,
    CaptureStreamType streamType
    )
    : _state(State::Created)
    , _streamType(streamType)
    , _capture(capture)
{
    Logger.CaptureReader_LifeTimeStart((void*)this);

    _mediaSink = Make<MediaSink>(
        profile->Audio, 
        profile->Video, 
        ref new MediaSampleHandler(this, &CaptureReader::ProcessAudioSample),
        ref new MediaSampleHandler(this, &CaptureReader::ProcessVideoSample)
        );
    CHKOOM(_mediaSink);

    _mediaExtension = reinterpret_cast<IMediaExtension^>(static_cast<AWM::IMediaExtension*>(_mediaSink.Get()));
}

CaptureReader::~CaptureReader()
{
    if (_state == State::Started)
    {
        if (_streamType == CaptureStreamType::Preview)
        {
            (void)_capture->StopPreviewAsync();
        }
        else
        {
            (void)_capture->StopRecordAsync();
        }
    }

    if (_mediaSink != nullptr)
    {
        (void)_mediaSink->Shutdown();
        _mediaSink = nullptr;
    }
    _mediaExtension = nullptr;
    _capture = nullptr;

    Logger.CaptureReader_LifeTimeStop((void*)this);
}

IAsyncAction^ CaptureReader::FinishAsync()
{
    auto lock = _lock.LockExclusive();

    if (_state != State::Started)
    {
        throw ref new COMException(E_UNEXPECTED, L"State");
    }
    _state = State::Closing;

    if (_mediaSink != nullptr)
    {
        (void)_mediaSink->Shutdown();
        _mediaSink = nullptr;
    }
    _mediaExtension = nullptr;

    task<void> task;
    if (_streamType == CaptureStreamType::Preview)
    {
        task = create_task(_capture->StopPreviewAsync());
    }
    else
    {
        task = create_task(_capture->StopRecordAsync());
    }

    return create_async([this, task]()
    {
        return task.then([this]()
        {
            auto lock = _lock.LockExclusive();
            _state = State::Closed;
            _capture = nullptr;
        });
    });
}

IAsyncOperation<IMediaSample^>^ CaptureReader::GetAudioSampleAsync()
{
    auto lock = _lock.LockExclusive();

    if (_state != State::Started)
    {
        throw ref new COMException(E_UNEXPECTED, L"State");
    }

    _mediaSink->RequestAudioSample();

    task_completion_event<IMediaSample^> taskEvent;
    _audioSampleRequestQueue.push(taskEvent);

    return create_async([taskEvent]()
    {
        return create_task(taskEvent);
    });
}

IAsyncOperation<IMediaSample^>^ CaptureReader::GetVideoSampleAsync()
{
    auto lock = _lock.LockExclusive();

    if (_state != State::Started)
    {
        throw ref new COMException(E_UNEXPECTED, L"State");
    }

    _mediaSink->RequestVideoSample();

    task_completion_event<IMediaSample^> taskEvent;
    _videoSampleRequestQueue.push(taskEvent);

    return create_async([taskEvent]()
    {
        return create_task(taskEvent);
    });
}

void CaptureReader::ProcessAudioSample(_In_ IMediaSample^ sample)
{
    task_completion_event<IMediaSample^> t;

    assert(sample != nullptr);

    {
        auto lock = _lock.LockExclusive();

        t = _audioSampleRequestQueue.front();
        _videoSampleRequestQueue.pop();
    }

    Logger.CaptureReader_AudioSample((void*)sample);

    // Dispatch without the lock taken to avoid deadlocks
    t.set(sample);
}

void CaptureReader::ProcessVideoSample(_In_ IMediaSample^ sample)
{
    task_completion_event<IMediaSample^> t;

    assert(sample != nullptr);

    {
        auto lock = _lock.LockExclusive();

        t = _videoSampleRequestQueue.front();
        _videoSampleRequestQueue.pop();
    }

    Logger.CaptureReader_VideoSample((void*)sample);

    // Dispatch without the lock taken to avoid deadlocks
    t.set(sample);
}
