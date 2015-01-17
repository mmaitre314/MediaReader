#pragma once

namespace MediaCaptureReader
{

    ref class MediaReaderVideoStream;
    ref class MediaReaderAudioStream;
    ref class MediaReaderOtherStream;
    ref struct MediaReaderInitializationSettings;
    class SourceReaderCallback;

    private ref class MediaReaderSharedState sealed : public IReaderSharedState
    {
    public:

        void CreateStreams(
            WFC::IVectorView<MediaReaderAudioStream^>^* audioStreams,
            WFC::IVectorView<MediaReaderVideoStream^>^* videoStreams,
            WFC::IVectorView<MediaReaderOtherStream^>^* otherStreams
            );

        void GetMetadata(WF::TimeSpan* duration, bool* canSeek);

        virtual WF::IAsyncAction^ CompleteInitializationAsync()
        {
            return concurrency::create_async([]()
            {
                return concurrency::task_from_result();
            });
        }

        virtual void Seek(WF::TimeSpan position);

        virtual WF::IAsyncAction^ FinishAsync();

        virtual MediaGraphicsDevice^ GetGraphicsDevice()
        {
            return _graphicsDevice;
        }

        WF::IAsyncOperation<MediaReaderReadResult^>^ ReadAsync(unsigned int streamIndex);

        virtual WF::IAsyncOperation<MediaReaderReadResult^>^ ReadAudioAsync(unsigned int streamIndex)
        {
            return ReadAsync(streamIndex);
        }
        virtual WF::IAsyncOperation<MediaReaderReadResult^>^ ReadVideoAsync(unsigned int streamIndex)
        {
            return ReadAsync(streamIndex);
        }
        virtual WF::IAsyncOperation<MediaReaderReadResult^>^ ReadOtherAsync(unsigned int streamIndex)
        {
            return ReadAsync(streamIndex);
        }

        virtual WMMp::AudioEncodingProperties^ GetCurrentAudioStreamProperties(unsigned int streamIndex);
        virtual WMMp::VideoEncodingProperties^ GetCurrentVideoStreamProperties(unsigned int streamIndex);
        virtual WMMp::IMediaEncodingProperties^ GetCurrentOtherStreamProperties(unsigned int streamIndex);

        virtual WFC::IVectorView<WMMp::AudioEncodingProperties^>^ GetNativeAudioStreamProperties(unsigned int streamIndex);
        virtual WFC::IVectorView<WMMp::VideoEncodingProperties^>^ GetNativeVideoStreamProperties(unsigned int streamIndex);
        virtual WFC::IVectorView<WMMp::IMediaEncodingProperties^>^ GetNativeOtherStreamProperties(unsigned int streamIndex);

        WF::IAsyncAction^ SetCurrentStreamPropertiesAsync(unsigned int streamIndex, ::Windows::Media::MediaProperties::IMediaEncodingProperties^ properties);

        virtual WF::IAsyncAction^ SetCurrentAudioStreamPropertiesAsync(unsigned int streamIndex, WMMp::AudioEncodingProperties^ properties)
        {
            return SetCurrentStreamPropertiesAsync(streamIndex, properties);
        }
        virtual WF::IAsyncAction^ SetCurrentVideoStreamPropertiesAsync(unsigned int streamIndex, WMMp::VideoEncodingProperties^ properties)
        {
            return SetCurrentStreamPropertiesAsync(streamIndex, properties);
        }
        virtual WF::IAsyncAction^ SetCurrentOtherStreamPropertiesAsync(unsigned int streamIndex, WMMp::IMediaEncodingProperties^ properties)
        {
            return SetCurrentStreamPropertiesAsync(streamIndex, properties);
        }

        WF::IAsyncAction^ SetNativeStreamPropertiesAsync(unsigned int streamIndex, WMMp::IMediaEncodingProperties^ properties);

        virtual WF::IAsyncAction^ SetNativeAudioStreamPropertiesAsync(unsigned int streamIndex, WMMp::AudioEncodingProperties^ properties)
        {
            return SetNativeStreamPropertiesAsync(streamIndex, properties);
        }
        virtual WF::IAsyncAction^ SetNativeVideoStreamPropertiesAsync(unsigned int streamIndex, WMMp::VideoEncodingProperties^ properties)
        {
            return SetNativeStreamPropertiesAsync(streamIndex, properties);
        }
        virtual WF::IAsyncAction^ SetNativeOtherStreamPropertiesAsync(unsigned int streamIndex, WMMp::IMediaEncodingProperties^ properties)
        {
            return SetNativeStreamPropertiesAsync(streamIndex, properties);
        }

        void SetSelection(unsigned int streamIndex, bool selected);

        virtual void SetAudioSelection(unsigned int streamIndex, bool selected)
        {
            SetSelection(streamIndex, selected);
        }
        virtual void SetVideoSelection(unsigned int streamIndex, bool selected)
        {
            SetSelection(streamIndex, selected);
        }
        virtual void SetOtherSelection(unsigned int streamIndex, bool selected)
        {
            SetSelection(streamIndex, selected);
        }

        bool GetSelection(unsigned int streamIndex);

        virtual bool GetAudioSelection(unsigned int streamIndex)
        {
            return GetSelection(streamIndex);
        }
        virtual bool GetVideoSelection(unsigned int streamIndex)
        {
            return GetSelection(streamIndex);
        }
        virtual bool GetOtherSelection(unsigned int streamIndex)
        {
            return GetSelection(streamIndex);
        }

    internal:

        MediaReaderSharedState(const Microsoft::WRL::ComPtr<IUnknown>& source, MediaReaderInitializationSettings^ settings);

        void OnReadSample(
            HRESULT hr,
            DWORD streamIndex,
            MF_SOURCE_READER_FLAG streamFlags,
            LONGLONG time,
            _In_opt_  IMFSample *sample
            );

        const MW::ComPtr<IMFSourceReaderEx>& SourceReader()
        {
            return _sourceReader;
        }

    private:

        ~MediaReaderSharedState();

        void _VerifyNotClosed()
        {
            if (_closed)
            {
                throw ref new Platform::ObjectDisposedException();
            }
        }

        MediaGraphicsDevice^ _GetSharedGraphicsDevice(MediaReaderInitializationSettings^ settings) const;
        MW::ComPtr<IMFAttributes> _CreateSourceReaderAttributes(
            MediaReaderInitializationSettings^ settings,
            MediaGraphicsDevice^ graphicsDevice,
            const Microsoft::WRL::ComPtr<SourceReaderCallback>& callback
            ) const;

        std::vector<std::queue<concurrency::task_completion_event<MediaReaderReadResult^>>> _sampleRequestQueues;
        MW::ComPtr<IMFSourceReaderEx> _sourceReader;
        MediaGraphicsDevice^ _graphicsDevice; // null if not HW accelerated
        unsigned int _streamCount;
        bool _closed;
        AutoMF _mf;
        MW::Wrappers::SRWLock _lock;
    };

}