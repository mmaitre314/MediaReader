#pragma once

namespace MediaCaptureReader
{
    ref class MediaGraphicsDevice;
    ref class MediaBuffer2D;

    public interface class IMediaSample
    {
        property WF::TimeSpan Timestamp;
        property WF::TimeSpan Duration;
    };

    public enum class BufferAccessMode
    {
        Read,
        Write,
        ReadWrite
    };

    public ref class MediaSample1D sealed : IMediaSample
    {
    public:

        virtual property WF::TimeSpan Timestamp;    // TODO: setting that value should update _sample
        virtual property WF::TimeSpan Duration;     // TODO: setting that value should update _sample

        // TODO: MediaBuffer1D^ LockBuffer(_In_ BufferAccessMode accessMode);

        // IClosable
        virtual ~MediaSample1D();

    internal:

        MediaSample1D(_In_ const MW::ComPtr<IMFSample>& sample);

        MW::ComPtr<IMFSample> GetSample() const
        {
            return _sample;
        }

    private:

        MW::ComPtr<IMFSample> _sample;
    };

    public enum class MediaSample2DFormat
    {
        Unknown,
        Nv12,
        Bgra8,
        Yuy2,
        Yv12
    };

    public ref class MediaSample2D sealed : IMediaSample
    {
    public:

        virtual property WF::TimeSpan Timestamp;    // TODO: setting that value should update _sample
        virtual property WF::TimeSpan Duration;     // TODO: setting that value should update _sample

        property MediaSample2DFormat Format { MediaSample2DFormat get() { return _format; } }
        property int Width { int get() { return _width; } }
        property int Height { int get() { return _height; } }
        property MediaGraphicsDevice^ GraphicsDevice { MediaGraphicsDevice^ get() { return _graphicsDevice; } }

        ///<summary>Allocate a new 2D media sample in software memory</summary>
        MediaSample2D(_In_ MediaSample2DFormat format, _In_ int width, _In_ int height);

        ///<summary>Get access to the sample pixel data</summary>
        MediaBuffer2D^ LockBuffer(_In_ BufferAccessMode accessMode);

        // IClosable
        virtual ~MediaSample2D();

    internal:

        MediaSample2D(
            _In_ const MW::ComPtr<IMFSample>& sample, 
            _In_ MediaSample2DFormat format, 
            _In_ int width, 
            _In_ int height, 
            _In_opt_ MediaGraphicsDevice^ graphicsDevice = nullptr
            );

        MW::ComPtr<IMFSample> GetSample() const
        {
            return _sample;
        }

        static MediaSample2DFormat GetFormatFromSubType(_In_ const GUID& subtype)
        {
            if (subtype == MFVideoFormat_NV12)
            {
                return MediaSample2DFormat::Nv12;
            }
            if ((subtype == MFVideoFormat_RGB32) || (subtype == MFVideoFormat_ARGB32))
            {
                return MediaSample2DFormat::Bgra8;
            }
            if (subtype == MFVideoFormat_YUY2)
            {
                return MediaSample2DFormat::Yuy2;
            }
            if (subtype == MFVideoFormat_YV12)
            {
                return MediaSample2DFormat::Yv12;
            }

            return MediaSample2DFormat::Unknown;
        }

        static uint32 GetFourCcFromFormat(_In_ MediaSample2DFormat format)
        {
            switch (format)
            {
            case MediaSample2DFormat::Nv12: return MFVideoFormat_NV12.Data1;
            case MediaSample2DFormat::Bgra8: return MFVideoFormat_RGB32.Data1;
            case MediaSample2DFormat::Yuy2: return MFVideoFormat_YUY2.Data1;
            case MediaSample2DFormat::Yv12: return MFVideoFormat_YV12.Data1;
            default: return 0;
            }
        }

    private:

        unsigned int _GetDefaultPitch() const;

        MediaSample2DFormat _format;
        int _width;
        int _height;
        MediaGraphicsDevice^ _graphicsDevice;

        MW::ComPtr<IMFSample> _sample;

        mutable MWW::SRWLock _lock;
    };
}

