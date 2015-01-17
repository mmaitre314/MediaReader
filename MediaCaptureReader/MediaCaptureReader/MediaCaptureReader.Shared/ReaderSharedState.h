#pragma once

namespace MediaCaptureReader
{

    ref class MediaReaderVideoStream;
    ref class MediaReaderAudioStream;
    ref class MediaReaderOtherStream;
    ref class MediaReaderReadResult;
    ref class MediaGraphicsDevice;

    public enum class CaptureStreamType
    {
        Preview = 0,
        Record
    };

    // <summary>Less-frequently-used initialization settings</summary>
    public ref struct MediaReaderCaptureInitializationSettings sealed
    {
        ///<summary>Default: Preview</summary>
        property CaptureStreamType Stream;

        MediaReaderCaptureInitializationSettings()
        {
            Stream = CaptureStreamType::Preview;
        }
    };

    public interface class IReaderSharedState
    {
    public:

        WF::IAsyncAction^ CompleteInitializationAsync();

        void Seek(WF::TimeSpan position);

        WF::IAsyncAction^ FinishAsync();

        MediaGraphicsDevice^ GetGraphicsDevice();

        WF::IAsyncOperation<MediaReaderReadResult^>^ ReadAudioAsync(unsigned int streamIndex);
        WF::IAsyncOperation<MediaReaderReadResult^>^ ReadVideoAsync(unsigned int streamIndex);
        WF::IAsyncOperation<MediaReaderReadResult^>^ ReadOtherAsync(unsigned int streamIndex);

        WMMp::AudioEncodingProperties^ GetCurrentAudioStreamProperties(unsigned int streamIndex);
        WMMp::VideoEncodingProperties^ GetCurrentVideoStreamProperties(unsigned int streamIndex);
        WMMp::IMediaEncodingProperties^ GetCurrentOtherStreamProperties(unsigned int streamIndex);

        WFC::IVectorView<WMMp::AudioEncodingProperties^>^ GetNativeAudioStreamProperties(unsigned int streamIndex);
        WFC::IVectorView<WMMp::VideoEncodingProperties^>^ GetNativeVideoStreamProperties(unsigned int streamIndex);
        WFC::IVectorView<WMMp::IMediaEncodingProperties^>^ GetNativeOtherStreamProperties(unsigned int streamIndex);

        WF::IAsyncAction^ SetCurrentAudioStreamPropertiesAsync(unsigned streamIndex, WMMp::AudioEncodingProperties^ properties);
        WF::IAsyncAction^ SetCurrentVideoStreamPropertiesAsync(unsigned streamIndex, WMMp::VideoEncodingProperties^ properties);
        WF::IAsyncAction^ SetCurrentOtherStreamPropertiesAsync(unsigned streamIndex, WMMp::IMediaEncodingProperties^ properties);

        WF::IAsyncAction^ SetNativeAudioStreamPropertiesAsync(unsigned int streamIndex, WMMp::AudioEncodingProperties^ properties);
        WF::IAsyncAction^ SetNativeVideoStreamPropertiesAsync(unsigned int streamIndex, WMMp::VideoEncodingProperties^ properties);
        WF::IAsyncAction^ SetNativeOtherStreamPropertiesAsync(unsigned int streamIndex, WMMp::IMediaEncodingProperties^ properties);

        void SetAudioSelection(unsigned int streamIndex, bool selected);
        void SetVideoSelection(unsigned int streamIndex, bool selected);
        void SetOtherSelection(unsigned int streamIndex, bool selected);

        bool GetAudioSelection(unsigned int streamIndex);
        bool GetVideoSelection(unsigned int streamIndex);
        bool GetOtherSelection(unsigned int streamIndex);
    };

}