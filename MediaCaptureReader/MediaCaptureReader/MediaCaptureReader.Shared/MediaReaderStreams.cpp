#include "pch.h"
#include "ReaderSharedState.h"
#include "MediaReaderReadResult.h"
#include "MediaReaderSharedState.h"
#include "MediaReaderStreams.h"
#include "MediaReader.h"
#include <windows.media.mediaproperties.h>

using namespace Microsoft::WRL;
using namespace concurrency;
using namespace MediaCaptureReader;
using namespace Platform;
using namespace Platform::Collections;
using namespace Windows::Foundation;
using namespace Windows::Foundation::Collections;
using namespace Windows::Storage::Streams;
using namespace Windows::Media::Core;
using namespace Windows::Media::MediaProperties;

//
// MediaReaderAudioStream
//

MediaReaderAudioStream::MediaReaderAudioStream(unsigned int streamIndex, IReaderSharedState^ state)
: _streamIndex(streamIndex)
, _state(state)
{
}

void MediaReaderAudioStream::Close()
{
    auto lock = _lock.LockExclusive();
    _state = nullptr;
}

IAsyncOperation<MediaReaderReadResult^>^ MediaReaderAudioStream::ReadAsync()
{
    auto lock = _lock.LockExclusive();
    _VerifyNotClosed();
    return _state->ReadAudioAsync(_streamIndex);
}

AudioEncodingProperties^ MediaReaderAudioStream::GetCurrentStreamProperties()
{
    auto lock = _lock.LockExclusive();
    _VerifyNotClosed();
    return _state->GetCurrentAudioStreamProperties(_streamIndex);
}

IAsyncAction^ MediaReaderAudioStream::SetCurrentStreamPropertiesAsync(AudioEncodingProperties^ properties)
{
    auto lock = _lock.LockExclusive();
    _VerifyNotClosed();
    return _state->SetCurrentAudioStreamPropertiesAsync(_streamIndex, properties);
}

IVectorView<AudioEncodingProperties^>^ MediaReaderAudioStream::GetNativeStreamProperties()
{
    auto lock = _lock.LockExclusive();
    _VerifyNotClosed();
    return _state->GetNativeAudioStreamProperties(_streamIndex);
}

IAsyncAction^ MediaReaderAudioStream::SetNativeStreamPropertiesAsync(AudioEncodingProperties^ properties)
{
    auto lock = _lock.LockExclusive();
    _VerifyNotClosed();
    return _state->SetNativeAudioStreamPropertiesAsync(_streamIndex, properties);
}

void MediaReaderAudioStream::SetSelection(bool selected)
{
    auto lock = _lock.LockExclusive();
    _VerifyNotClosed();
    _state->SetAudioSelection(_streamIndex, selected);
}

bool MediaReaderAudioStream::IsSelected::get()
{
    return _state == nullptr ? false : _state->GetAudioSelection(_streamIndex);
}

//
// MediaReaderVideoStream
//

MediaReaderVideoStream::MediaReaderVideoStream(unsigned int streamIndex, IReaderSharedState^ state)
: _streamIndex(streamIndex)
, _state(state)
{
}

void MediaReaderVideoStream::Close()
{
    auto lock = _lock.LockExclusive();
    _state = nullptr;
}

IAsyncOperation<MediaReaderReadResult^>^ MediaReaderVideoStream::ReadAsync()
{
    auto lock = _lock.LockExclusive();
    _VerifyNotClosed();
    return _state->ReadVideoAsync(_streamIndex);
}

VideoEncodingProperties^ MediaReaderVideoStream::GetCurrentStreamProperties()
{
    auto lock = _lock.LockExclusive();
    _VerifyNotClosed();
    return _state->GetCurrentVideoStreamProperties(_streamIndex);
}

IAsyncAction^ MediaReaderVideoStream::SetCurrentStreamPropertiesAsync(VideoEncodingProperties^ properties)
{
    auto lock = _lock.LockExclusive();
    _VerifyNotClosed();
    return _state->SetCurrentVideoStreamPropertiesAsync(_streamIndex, properties);
}

IVectorView<VideoEncodingProperties^>^ MediaReaderVideoStream::GetNativeStreamProperties()
{
    auto lock = _lock.LockExclusive();
    _VerifyNotClosed();
    return _state->GetNativeVideoStreamProperties(_streamIndex);
}

IAsyncAction^ MediaReaderVideoStream::SetNativeStreamPropertiesAsync(VideoEncodingProperties^ properties)
{
    auto lock = _lock.LockExclusive();
    _VerifyNotClosed();
    return _state->SetNativeVideoStreamPropertiesAsync(_streamIndex, properties);
}

void MediaReaderVideoStream::SetSelection(bool selected)
{
    auto lock = _lock.LockExclusive();
    _VerifyNotClosed();
    _state->SetVideoSelection(_streamIndex, selected);
}

bool MediaReaderVideoStream::IsSelected::get()
{
    return _state == nullptr ? false : _state->GetVideoSelection(_streamIndex);
}

//
// MediaReaderOtherStream
//

MediaReaderOtherStream::MediaReaderOtherStream(unsigned int streamIndex, IReaderSharedState^ state)
: _streamIndex(streamIndex)
, _state(state)
{
}

void MediaReaderOtherStream::Close()
{
    auto lock = _lock.LockExclusive();
    _state = nullptr;
}

IAsyncOperation<MediaReaderReadResult^>^ MediaReaderOtherStream::ReadAsync()
{
    auto lock = _lock.LockExclusive();
    _VerifyNotClosed();
    return _state->ReadOtherAsync(_streamIndex);
}

IMediaEncodingProperties^ MediaReaderOtherStream::GetCurrentStreamProperties()
{
    auto lock = _lock.LockExclusive();
    _VerifyNotClosed();
    return _state->GetCurrentOtherStreamProperties(_streamIndex);
}

IAsyncAction^ MediaReaderOtherStream::SetCurrentStreamPropertiesAsync(IMediaEncodingProperties^ properties)
{
    auto lock = _lock.LockExclusive();
    _VerifyNotClosed();
    return _state->SetCurrentOtherStreamPropertiesAsync(_streamIndex, properties);
}

IVectorView<IMediaEncodingProperties^>^ MediaReaderOtherStream::GetNativeStreamProperties()
{
    auto lock = _lock.LockExclusive();
    _VerifyNotClosed();
    return _state->GetNativeOtherStreamProperties(_streamIndex);
}

IAsyncAction^ MediaReaderOtherStream::SetNativeStreamPropertiesAsync(IMediaEncodingProperties^ properties)
{
    auto lock = _lock.LockExclusive();
    _VerifyNotClosed();
    return _state->SetNativeOtherStreamPropertiesAsync(_streamIndex, properties);
}

void MediaReaderOtherStream::SetSelection(bool selected)
{
    auto lock = _lock.LockExclusive();
    _VerifyNotClosed();
    _state->SetOtherSelection(_streamIndex, selected);
}

bool MediaReaderOtherStream::IsSelected::get()
{
    return _state == nullptr ? false : _state->GetOtherSelection(_streamIndex);
}
