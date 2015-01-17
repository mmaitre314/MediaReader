#pragma once

namespace MediaCaptureReader
{
    ref class MediaGraphicsDevice;

    public enum class MediaSampleFormat
    {
        Unknown,
        Nv12,
        Bgra8,
        Yuy2,
        Yv12
    };

    public ref class MediaSample sealed
    {
    public:

        property MediaSampleFormat Format;
        property WF::TimeSpan Timestamp;
        property WF::TimeSpan Duration;

        // Only for 2D samples
        property int Width;
        property int Height;
        property MediaGraphicsDevice^ GraphicsDevice;

        // IClosable
        virtual ~MediaSample();

    internal:

        MediaSample(_In_ const MW::ComPtr<IMFSample>& sample);

        MW::ComPtr<IMFSample> GetSample() const
        {
            return _sample;
        }

        static MediaSampleFormat GetFormatFromMfSubType(_In_ const GUID& subtype)
        {
            if (subtype == MFVideoFormat_NV12)
            {
                return MediaSampleFormat::Nv12;
            }
            if ((subtype == MFVideoFormat_RGB32) || (subtype == MFVideoFormat_ARGB32))
            {
                return MediaSampleFormat::Bgra8;
            }
            if (subtype == MFVideoFormat_YUY2)
            {
                return MediaSampleFormat::Yuy2;
            }
            if (subtype == MFVideoFormat_YV12)
            {
                return MediaSampleFormat::Yv12;
            }

            return MediaSampleFormat::Unknown;
        }

    private:

        MW::ComPtr<IMFSample> _sample;
    };
}

