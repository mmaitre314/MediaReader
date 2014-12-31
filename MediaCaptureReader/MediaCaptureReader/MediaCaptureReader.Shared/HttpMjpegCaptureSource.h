#pragma once

namespace MediaCaptureReader
{
    public ref class HttpMjpegCaptureSource sealed
    {
    public:

        static WF::IAsyncOperation<HttpMjpegCaptureSource^>^ CreateFromUriAsync(_In_ WF::Uri^ uri);
        static WF::IAsyncOperation<HttpMjpegCaptureSource^>^ CreateFromStreamAsync(_In_ WSS::IInputStream^ stream, _In_ Platform::String^ boundary);

        // IClosable
        virtual ~HttpMjpegCaptureSource();

        property WMCo::MediaStreamSource^ Source 
        { 
            WMCo::MediaStreamSource^ get()
            {
                return _source;
            }
        }

    private:

        HttpMjpegCaptureSource();

        concurrency::task<void> _InitializeAsync();

        void _ReadFramesAsync();
        concurrency::task<WSS::IBuffer^> _ReadSingleFrameAsync();

        void _QueueRequest(_In_ WMCo::MediaStreamSourceSampleRequest ^request);
        void _QueueSample(_In_ WMCo::MediaStreamSample^ sample);
        void _ProcessQueue();

#if WINDOWS_PHONE_APP
        const bool _decodeMJPEG = true; // No MJPEG decoder
#else
        const bool _decodeMJPEG = true; // MJPEG decoder in-box
#endif

        std::string _httpBoundary;
        WSS::IInputStream^ _stream;
        WMCo::MediaStreamSource^ _source;
        unsigned int _frameRate;
        WSS::Buffer^ _streamReadBuffer;
        std::vector<unsigned char> _accumulationBuffer;
        MW::ComPtr<IWICImagingFactory> _wicFactory;
        MFTIME _timeOffset;
        bool _started;
        bool _closed;

        // Sample/request queue
        std::queue<WMCo::MediaStreamSample^> _samples;
        std::queue<WMCo::MediaStreamSourceSampleRequest^> _sampleRequests;
        std::queue<WMCo::MediaStreamSourceSampleRequestDeferral^> _sampleRequestDeferrals;
        bool _discontinuity;

        MWW::SRWLock _lock;
    };
}
