#pragma once

namespace MediaCaptureReader
{
    ref class MediaGraphicsDevice;

    public ref class MediaSample sealed
    {
    public:

        property WF::TimeSpan Timestamp;
        property WF::TimeSpan Duration;

        // Only for 2D samples
        property uint32 Format;
        property uint32 Width;
        property uint32 Height;
        property MediaGraphicsDevice^ GraphicsDevice;

        // IClosable
        virtual ~MediaSample();

    internal:

        MediaSample(_In_ const MW::ComPtr<IMFSample>& sample);

        MW::ComPtr<IMFSample> GetSample() const
        {
            return _sample;
        }

    private:

        MW::ComPtr<IMFSample> _sample;
    };
}

