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

    private:

        MW::ComPtr<IMFSample> _sample;
    };
}

