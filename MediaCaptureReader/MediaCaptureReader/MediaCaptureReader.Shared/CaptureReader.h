#pragma once

class MediaSink;

namespace MediaCaptureReader
{
    ref class MediaSample;

    public enum class CaptureStreamType
    {
        Preview = 0,
        Record
    };

    public ref class CaptureReader sealed
    {
    public:

        // IClosable
        virtual ~CaptureReader();

        ///<summary>Creates a MediaCaptureReader reading samples from the preview stream.</summary>
        static WF::IAsyncOperation<CaptureReader^>^ CreateAsync(
            _In_ WMC::MediaCapture^ capture, 
            _In_ WMMp::MediaEncodingProfile^ profile
            )
        {
            return CreateAsync(capture, profile, CaptureStreamType::Preview);
        }

        static WF::IAsyncOperation<CaptureReader^>^ CreateAsync(
            _In_ WMC::MediaCapture^ capture, 
            _In_ WMMp::MediaEncodingProfile^ profile,
            _In_ CaptureStreamType streamType
            );

        WF::IAsyncOperation<MediaSample^>^ GetAudioSampleAsync();
        WF::IAsyncOperation<MediaSample^>^ GetVideoSampleAsync();

        WF::IAsyncAction^ FinishAsync();

    private:

        CaptureReader(
            _In_ WMC::MediaCapture^ capture, 
            _In_ WMMp::MediaEncodingProfile^ profile,
            CaptureStreamType streamType
            );

        void ProcessAudioSample(_In_ MediaSample^ sample);
        void ProcessVideoSample(_In_ MediaSample^ sample);

        Platform::Agile<WMC::MediaCapture> _capture;
        WM::IMediaExtension^ _mediaExtension;

        MW::ComPtr<MediaSink> _mediaSink;

        CaptureStreamType _streamType;

        enum class State
        {
            Created,
            Started,
            Closing,
            Closed
        } _state;

        std::queue<concurrency::task_completion_event<MediaSample^>> _audioSampleRequestQueue;
        std::queue<concurrency::task_completion_event<MediaSample^>> _videoSampleRequestQueue;

        AutoMF _mf;
        MWW::SRWLock _lock;
    };

}