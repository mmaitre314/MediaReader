#pragma once

namespace MediaCaptureReader
{
    ref class MediaReaderVideoStream;
    ref class MediaReaderAudioStream;
    ref class MediaReaderOtherStream;
    interface class IReaderSharedState;

    public interface class IMediaReaderStream
    {
        WF::IAsyncOperation<MediaReaderReadResult^>^ ReadAsync();

        void SetSelection(bool selected);
        property bool IsSelected { bool get(); }
    };

    //
    // MediaReaderAudioStream
    //

    public ref class MediaReaderAudioStream sealed : public IMediaReaderStream
    {
    public:

        virtual WF::IAsyncOperation<MediaReaderReadResult^>^ ReadAsync();

        WMMp::AudioEncodingProperties^ GetCurrentStreamProperties();
        WF::IAsyncAction^ SetCurrentStreamPropertiesAsync(WMMp::AudioEncodingProperties^ properties);

        WFC::IVectorView<WMMp::AudioEncodingProperties^>^ GetNativeStreamProperties();
        WF::IAsyncAction^ SetNativeStreamPropertiesAsync(WMMp::AudioEncodingProperties^ properties);

        virtual void SetSelection(bool selected);
        virtual property bool IsSelected { bool get(); }

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

    public ref class MediaReaderVideoStream sealed : public IMediaReaderStream
    {
    public:

        virtual WF::IAsyncOperation<MediaReaderReadResult^>^ ReadAsync();

        WMMp::VideoEncodingProperties^ GetCurrentStreamProperties();
        WF::IAsyncAction^ SetCurrentStreamPropertiesAsync(WMMp::VideoEncodingProperties^ properties);

        WFC::IVectorView<WMMp::VideoEncodingProperties^>^ GetNativeStreamProperties();
        WF::IAsyncAction^ SetNativeStreamPropertiesAsync(WMMp::VideoEncodingProperties^ properties);

        virtual void SetSelection(bool selected);
        virtual property bool IsSelected { bool get(); }

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

    public ref class MediaReaderOtherStream sealed : public IMediaReaderStream
    {
    public:

        virtual WF::IAsyncOperation<MediaReaderReadResult^>^ ReadAsync();

        WMMp::IMediaEncodingProperties^ GetCurrentStreamProperties();
        WF::IAsyncAction^ SetCurrentStreamPropertiesAsync(WMMp::IMediaEncodingProperties^ properties);

        WFC::IVectorView<WMMp::IMediaEncodingProperties^>^ GetNativeStreamProperties();
        WF::IAsyncAction^ SetNativeStreamPropertiesAsync(WMMp::IMediaEncodingProperties^ properties);

        virtual void SetSelection(bool selected);
        virtual property bool IsSelected { bool get(); }

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
