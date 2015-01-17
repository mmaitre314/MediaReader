#pragma once

namespace MediaCaptureReader
{
    ref class MediaReaderVideoStream;
    ref class MediaReaderAudioStream;
    ref class MediaReaderOtherStream;
    interface class IReaderSharedState;

    //
    // MediaReaderAudioStream
    //

    public ref class MediaReaderAudioStream sealed
    {
    public:

        WF::IAsyncOperation<MediaReaderReadResult^>^ ReadAsync();

        WMMp::AudioEncodingProperties^ GetCurrentStreamProperties();
        WF::IAsyncAction^ SetCurrentStreamPropertiesAsync(WMMp::AudioEncodingProperties^ properties);

        WFC::IVectorView<WMMp::AudioEncodingProperties^>^ GetNativeStreamProperties();
        WF::IAsyncAction^ SetNativeStreamPropertiesAsync(WMMp::AudioEncodingProperties^ properties);

        void SetSelection(bool selected);
        property bool IsSelected { bool get(); }

    internal:

        MediaReaderAudioStream(unsigned int streamIndex, IReaderSharedState^ state);

        void Close();

    private:

        void _VerifyNotClosed()
        {
            if (_state == nullptr)
            {
                throw ref new Platform::ObjectDisposedException();
            }
        }

        unsigned int _streamIndex;
        IReaderSharedState^ _state;
        MW::Wrappers::SRWLock _lock;
    };

    //
    // MediaReaderVideoStream
    //

    public ref class MediaReaderVideoStream sealed
    {
    public:

        ::Windows::Foundation::IAsyncOperation<MediaReaderReadResult^>^ ReadAsync();

        WMMp::VideoEncodingProperties^ GetCurrentStreamProperties();
        ::Windows::Foundation::IAsyncAction^ SetCurrentStreamPropertiesAsync(WMMp::VideoEncodingProperties^ properties);

        WFC::IVectorView<WMMp::VideoEncodingProperties^>^ GetNativeStreamProperties();
        ::Windows::Foundation::IAsyncAction^ SetNativeStreamPropertiesAsync(WMMp::VideoEncodingProperties^ properties);

        void SetSelection(bool selected);
        property bool IsSelected { bool get(); }

    internal:

        MediaReaderVideoStream(unsigned int streamIndex, IReaderSharedState^ state);

        void Close();

    private:

        void _VerifyNotClosed()
        {
            if (_state == nullptr)
            {
                throw ref new Platform::ObjectDisposedException();
            }
        }

        unsigned int _streamIndex;
        IReaderSharedState^ _state;
        Microsoft::WRL::Wrappers::SRWLock _lock;
    };

    //
    // MediaReaderOtherStream
    //

    public ref class MediaReaderOtherStream sealed
    {
    public:

        ::Windows::Foundation::IAsyncOperation<MediaReaderReadResult^>^ ReadAsync();

        WMMp::IMediaEncodingProperties^ GetCurrentStreamProperties();
        ::Windows::Foundation::IAsyncAction^ SetCurrentStreamPropertiesAsync(WMMp::IMediaEncodingProperties^ properties);

        WFC::IVectorView<WMMp::IMediaEncodingProperties^>^ GetNativeStreamProperties();
        ::Windows::Foundation::IAsyncAction^ SetNativeStreamPropertiesAsync(WMMp::IMediaEncodingProperties^ properties);

        void SetSelection(bool selected);
        property bool IsSelected { bool get(); }

    internal:

        MediaReaderOtherStream(unsigned int streamIndex, IReaderSharedState^ state);

        void Close();

    private:

        void _VerifyNotClosed()
        {
            if (_state == nullptr)
            {
                throw ref new Platform::ObjectDisposedException();
            }
        }

        unsigned int _streamIndex;
        IReaderSharedState^ _state;
        Microsoft::WRL::Wrappers::SRWLock _lock;
    };

}
