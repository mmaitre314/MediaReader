#pragma once

namespace MediaCaptureReader
{
    public ref class MediaSample sealed
    {
    public:

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

