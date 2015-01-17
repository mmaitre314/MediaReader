#pragma once

namespace MediaCaptureReader
{
    ref class MediaGraphicsDevice;

    public enum class AudioInitialization
    {
        Pcm = 0,
        PassThrough,
        Deselected
    };

    public enum class VideoInitialization
    {
        Nv12 = 0,
        Bgra8,
        PassThrough,
        Deselected
    };

    ///<summary>Less-frequently-used initialization settings</summary>
    public ref struct MediaReaderInitializationSettings sealed
    {
        ///<summary>Default: true</summary>
        property bool HardwareAccelerationEnabled;

        ///<summary>Default: true</summary>
        property bool ConvertersEnabled;

        ///<summary>Default: false</summary>
        property bool LowLatencyEnabled;

        property MediaGraphicsDevice^ GraphicsDevice;

        MediaReaderInitializationSettings()
        {
            HardwareAccelerationEnabled = true;
            ConvertersEnabled = true;
            LowLatencyEnabled = false;
        }
    };

    public ref class MediaReader sealed
    {
    public:

        //
        // Factories
        //

        /// <summary>
        /// Read an audio or video file. Use "ms-appx:///" to load a file from the app package and "ms-appdata:///" to load a file from the app state. 
        /// The default format for the video stream is NV12 and for the audio stream is PCM.
        /// </summary>
        static WF::IAsyncOperation<MediaReader^>^ CreateFromPathAsync(
            Platform::String^ absolutePath
            )
        {
            return MediaReader::CreateFromPathAsync(absolutePath, AudioInitialization::Pcm, VideoInitialization::Nv12, ref new MediaReaderInitializationSettings());
        }

        /// <summary>Read an audio or video file. Use "ms-appx:///" to load a file from the app package and "ms-appdata:///" to load a file from the app state.</summary>
        static WF::IAsyncOperation<MediaReader^>^ CreateFromPathAsync(
            Platform::String^ absolutePath,
            AudioInitialization audio,
            VideoInitialization video
            )
        {
            return MediaReader::CreateFromPathAsync(absolutePath, audio, video, ref new MediaReaderInitializationSettings());
        }

        /// <summary>Read an audio or video file. Use "ms-appx:///" to load a file from the app package and "ms-appdata:///" to load a file from the app state.</summary>
        static WF::IAsyncOperation<MediaReader^>^ CreateFromPathAsync(
            ::Platform::String^ absolutePath,
            AudioInitialization audio,
            VideoInitialization video,
            MediaReaderInitializationSettings^ settings
            );

        /// <summary>Read an audio or video stream. The default format for the video stream is NV12 and for the audio stream is PCM.</summary>
        static WF::IAsyncOperation<MediaReader^>^ CreateFromStreamAsync(
            WSS::IRandomAccessStream^ stream
            )
        {
            return MediaReader::CreateFromStreamAsync(stream, AudioInitialization::Pcm, VideoInitialization::Nv12, ref new MediaReaderInitializationSettings());
        }

        /// <summary>Read an audio or video bytestream.</summary>
        static WF::IAsyncOperation<MediaReader^>^ CreateFromStreamAsync(
            WSS::IRandomAccessStream^ stream,
            AudioInitialization audio,
            VideoInitialization video
            )
        {
            return MediaReader::CreateFromStreamAsync(stream, audio, video, ref new MediaReaderInitializationSettings());
        }

        /// <summary>Read an audio or video bytestream.</summary>
        static WF::IAsyncOperation<MediaReader^>^ CreateFromStreamAsync(
            WSS::IRandomAccessStream^ stream,
            AudioInitialization audio,
            VideoInitialization video,
            MediaReaderInitializationSettings^ settings
            );

        /// <summary>Read an audio or video file. The default format for the video stream is NV12 and for the audio stream is PCM.</summary>
        static WF::IAsyncOperation<MediaReader^>^ CreateFromFileAsync(
            WS::IStorageFile^ file
            )
        {
            return MediaReader::CreateFromFileAsync(file, AudioInitialization::Pcm, VideoInitialization::Nv12, ref new MediaReaderInitializationSettings());
        }

        /// <summary>Read an audio or video file.</summary>
        static WF::IAsyncOperation<MediaReader^>^ CreateFromFileAsync(
            WS::IStorageFile^ file,
            AudioInitialization audio,
            VideoInitialization video
            )
        {
            return MediaReader::CreateFromFileAsync(file, audio, video, ref new MediaReaderInitializationSettings());
        }

        /// <summary>Read an audio or video file.</summary>
        static WF::IAsyncOperation<MediaReader^>^ CreateFromFileAsync(
            WS::IStorageFile^ file,
            AudioInitialization audio,
            VideoInitialization video,
            MediaReaderInitializationSettings^ settings
            );

        /// <summary>Read audio and video data from a Media Source like MediaStreamSource for instance. The default format for the video stream is NV12 and for the audio stream is PCM.</summary>
        static WF::IAsyncOperation<MediaReader^>^ CreateFromMediaSourceAsync(
            ::Windows::Media::Core::IMediaSource^ source
            )
        {
            return MediaReader::CreateFromMediaSourceAsync(source, AudioInitialization::Pcm, VideoInitialization::Nv12, ref new MediaReaderInitializationSettings());
        }

        /// <summary>Read audio and video data from a Media Source like MediaStreamSource for instance.</summary>
        static WF::IAsyncOperation<MediaReader^>^ CreateFromMediaSourceAsync(
            ::Windows::Media::Core::IMediaSource^ source,
            AudioInitialization audio,
            VideoInitialization video
            )
        {
            return MediaReader::CreateFromMediaSourceAsync(source, audio, video, ref new MediaReaderInitializationSettings());
        }

        /// <summary>Read audio and video data from a Media Source like MediaStreamSource for instance.</summary>
        static WF::IAsyncOperation<MediaReader^>^ CreateFromMediaSourceAsync(
            ::Windows::Media::Core::IMediaSource^ source,
            AudioInitialization audio,
            VideoInitialization video,
            MediaReaderInitializationSettings^ settings
            );

        /// <summary>Read audio and video data from a microphone or camera using MediaCapture. The default format for the video stream is NV12 and for the audio stream is PCM.</summary>
        static WF::IAsyncOperation<MediaReader^>^ CreateFromMediaCaptureAsync(
            ::Windows::Media::Capture::MediaCapture^ capture
            )
        {
            return MediaReader::CreateFromMediaCaptureAsync(capture, AudioInitialization::Pcm, VideoInitialization::Nv12, ref new MediaReaderCaptureInitializationSettings());
        }

        /// <summary>Read audio and video data from a microphone or camera using MediaCapture.</summary>
        static WF::IAsyncOperation<MediaReader^>^ CreateFromMediaCaptureAsync(
            ::Windows::Media::Capture::MediaCapture^ capture,
            AudioInitialization audio,
            VideoInitialization video
            )
        {
            return MediaReader::CreateFromMediaCaptureAsync(capture, audio, video, ref new MediaReaderCaptureInitializationSettings());
        }

        /// <summary>Read audio and video data from a microphone or camera using MediaCapture.</summary>
        static WF::IAsyncOperation<MediaReader^>^ CreateFromMediaCaptureAsync(
            ::Windows::Media::Capture::MediaCapture^ capture,
            AudioInitialization audio,
            VideoInitialization video,
            MediaReaderCaptureInitializationSettings^ settings
            );

        //
        // Member methods
        //

        // IClosable
        virtual ~MediaReader();

        // TODO
        //// INativeService
        ///// <summary>Gives access to the underlying IMFSourceReaderEx via IMediaReaderNative. Returns null when created from MediaCapture.</summary>
        //virtual Platform::Object^ GetService(Platform::Guid service);

        void Seek(WF::TimeSpan position);

        /// <summary>The list of audio streams in the file. May be empty.</summary>
        property WF::Collections::IVectorView<MediaReaderAudioStream^>^ AudioStreams
        {
            WF::Collections::IVectorView<MediaReaderAudioStream^>^ get()
            {
                auto lock = _lock.LockExclusive();
                return _audioStreams;
            };
        }

        /// <summary>The list of video streams in the file. May be empty.</summary>
        property WF::Collections::IVectorView<MediaReaderVideoStream^>^ VideoStreams
        {
            WF::Collections::IVectorView<MediaReaderVideoStream^>^ get()
            {
                auto lock = _lock.LockExclusive();
                return _videoStreams;
            };
        }

        /// <summary>The list of non-audio/video streams in the file. May be empty.</summary>
        property WF::Collections::IVectorView<MediaReaderOtherStream^>^ OtherStreams
        {
            WF::Collections::IVectorView<MediaReaderOtherStream^>^ get()
            {
                auto lock = _lock.LockExclusive();
                return _otherStreams;
            };
        }

        /// <summary>The GraphicsDevice used to decode and process the video data. Null if only the CPU is used.</summary>
        property MediaGraphicsDevice^ GraphicsDevice
        {
            MediaGraphicsDevice^ get()
            {
                auto lock = _lock.LockExclusive();
                return _state == nullptr ? nullptr : _state->GetGraphicsDevice();
            }
        }

        /// <summary>The duration of the media. Zero if unknown.</summary>
        property WF::TimeSpan Duration
        {
            WF::TimeSpan get()
            {
                return _duration;
            }
        }

        property bool CanSeek
        {
            bool get()
            {
                return _canSeek;
            }
        }

    private:

        MediaReader(WSS::IRandomAccessStream^ stream, MediaReaderInitializationSettings^ settings);
        MediaReader(::Windows::Media::Core::IMediaSource^ source, MediaReaderInitializationSettings^ settings);
        MediaReader(::Windows::Media::Capture::MediaCapture^ capture, MediaReaderCaptureInitializationSettings^ settings);

        WF::IAsyncAction^ _InitializeAsync(
            AudioInitialization audio,
            VideoInitialization video
            );

        void _VerifyNotClosed()
        {
            if (_state == nullptr)
            {
                throw ref new Platform::ObjectDisposedException();
            }
        }

        WF::Collections::IVectorView<MediaReaderVideoStream^>^ _videoStreams;
        WF::Collections::IVectorView<MediaReaderAudioStream^>^ _audioStreams;
        WF::Collections::IVectorView<MediaReaderOtherStream^>^ _otherStreams;

        WF::TimeSpan _duration;
        bool _canSeek;

        IReaderSharedState^ _state;

        AutoMF _mf;
        Microsoft::WRL::Wrappers::SRWLock _lock;
    };

}

