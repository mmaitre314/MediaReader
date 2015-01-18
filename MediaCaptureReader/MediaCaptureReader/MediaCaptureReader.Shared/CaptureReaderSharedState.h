#pragma once

namespace MediaCaptureReader
{

    ref class MediaReaderVideoStream;
    ref class MediaReaderAudioStream;
    ref class MediaReaderOtherStream;
    ref struct MediaReaderCaptureInitializationSettings;
    ref class MediaReaderReadResult;
    class MediaSink;

    private ref class CaptureReaderSharedState sealed : public IReaderSharedState
    {
    public:

        virtual void CreateStreams(
            _Outptr_ WFC::IVectorView<MediaReaderAudioStream^>^* audioStreams,
            _Outptr_ WFC::IVectorView<MediaReaderVideoStream^>^* videoStreams,
            _Outptr_ WFC::IVectorView<MediaReaderOtherStream^>^* otherStreams
            );

        virtual WF::IAsyncAction^ CompleteInitializationAsync();

        virtual void Seek(WF::TimeSpan position)
        {
            throw ref new Platform::COMException(E_UNEXPECTED, L"Capture cannot seek");
        }

        virtual WF::IAsyncAction^ FinishAsync();

        virtual MediaGraphicsDevice^ GetGraphicsDevice()
        {
            return _graphicsDevice;
        }

        virtual WF::IAsyncOperation<MediaReaderReadResult^>^ ReadAudioAsync(unsigned int streamIndex);
        virtual WF::IAsyncOperation<MediaReaderReadResult^>^ ReadVideoAsync(unsigned int streamIndex);
        virtual WF::IAsyncOperation<MediaReaderReadResult^>^ ReadOtherAsync(unsigned int /*streamIndex*/)
        {
            RoFailFastWithErrorContext(E_UNEXPECTED);
            return nullptr;
        }

        virtual WMMp::AudioEncodingProperties^ GetCurrentAudioStreamProperties(unsigned int streamIndex);
        virtual WMMp::VideoEncodingProperties^ GetCurrentVideoStreamProperties(unsigned int streamIndex);
        virtual WMMp::IMediaEncodingProperties^ GetCurrentOtherStreamProperties(unsigned int /*streamIndex*/)
        {
            RoFailFastWithErrorContext(E_UNEXPECTED);
            return nullptr;
        }

        virtual WFC::IVectorView<WMMp::AudioEncodingProperties^>^ GetNativeAudioStreamProperties(unsigned int streamIndex);
        virtual WFC::IVectorView<WMMp::VideoEncodingProperties^>^ GetNativeVideoStreamProperties(unsigned int streamIndex);
        virtual WFC::IVectorView<WMMp::IMediaEncodingProperties^>^ GetNativeOtherStreamProperties(unsigned int /*streamIndex*/)
        {
            RoFailFastWithErrorContext(E_UNEXPECTED);
            return nullptr;
        }

        virtual WF::IAsyncAction^ SetCurrentAudioStreamPropertiesAsync(unsigned streamIndex, _In_ WMMp::AudioEncodingProperties^ properties);
        virtual WF::IAsyncAction^ SetCurrentVideoStreamPropertiesAsync(unsigned streamIndex, _In_ WMMp::VideoEncodingProperties^ properties);
        virtual WF::IAsyncAction^ SetCurrentOtherStreamPropertiesAsync(unsigned /*streamIndex*/, _In_ WMMp::IMediaEncodingProperties^ /*properties*/)
        {
            RoFailFastWithErrorContext(E_UNEXPECTED);
            return nullptr;
        }

        virtual WF::IAsyncAction^ SetNativeAudioStreamPropertiesAsync(unsigned int streamIndex, _In_ WMMp::AudioEncodingProperties^ properties);
        virtual WF::IAsyncAction^ SetNativeVideoStreamPropertiesAsync(unsigned int streamIndex, _In_ WMMp::VideoEncodingProperties^ properties);
        virtual WF::IAsyncAction^ SetNativeOtherStreamPropertiesAsync(unsigned int /*streamIndex*/, _In_ WMMp::IMediaEncodingProperties^ /*properties*/)
        {
            RoFailFastWithErrorContext(E_UNEXPECTED);
            return nullptr;
        }

        virtual void SetAudioSelection(unsigned int streamIndex, bool selected);
        virtual void SetVideoSelection(unsigned int streamIndex, bool selected);
        virtual void SetOtherSelection(unsigned int /*streamIndex*/, bool /*selected*/)
        {
            RoFailFastWithErrorContext(E_UNEXPECTED);
        }

        virtual bool GetAudioSelection(unsigned int streamIndex);
        virtual bool GetVideoSelection(unsigned int streamIndex);
        virtual bool GetOtherSelection(unsigned int /*streamIndex*/)
        {
            RoFailFastWithErrorContext(E_UNEXPECTED);
            return false;
        }

    internal:

        CaptureReaderSharedState(_In_ WMC::MediaCapture^ capture, _In_ MediaReaderCaptureInitializationSettings^ settings);

    private:

        ~CaptureReaderSharedState();

        void ProcessAudioSample(_In_ IMediaSample^ sample);
        void ProcessVideoSample(_In_ IMediaSample^ sample);

        void _VerifyNotClosed()
        {
            if ((_state == State::Closing) || (_state == State::Closed))
            {
                throw ref new Platform::ObjectDisposedException();
            }
        }

        WMC::MediaStreamType _GetVideoStreamType() const
        {
            switch (_streamType)
            {
            case CaptureStreamType::Preview: return WMC::MediaStreamType::VideoPreview;
            case CaptureStreamType::Record: return WMC::MediaStreamType::VideoRecord;
            default: RoFailFastWithErrorContext(E_UNEXPECTED); return WMC::MediaStreamType::Audio;
            }
        }

        enum class State
        {
            Created,
            Starting,
            Started,
            Closing,
            Closed
        } _state;

        CaptureStreamType _streamType;

        std::queue<concurrency::task_completion_event<MediaReaderReadResult^>> _audioSampleRequestQueue;
        std::queue<concurrency::task_completion_event<MediaReaderReadResult^>> _videoSampleRequestQueue;

        MW::ComPtr<MediaSink> _mediaSink;
        WM::MediaProperties::MediaEncodingProfile^ _encodingProfile;
        WM::IMediaExtension^ _mediaExtension;
        MediaGraphicsDevice^ _graphicsDevice; // null if not HW accelerated
        Platform::Agile<WMC::MediaCapture> _capture;
        unsigned int _streamCount;
        bool _audioSelected;
        bool _videoSelected;
        Microsoft::WRL::Wrappers::SRWLock _lock;
    };

}