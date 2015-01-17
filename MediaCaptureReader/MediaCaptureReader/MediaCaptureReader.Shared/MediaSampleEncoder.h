#pragma once

namespace MediaCaptureReader
{
    ref class MediaSample;

    public enum class ContainerFormat
    {
        Jpeg = 0
    };

    public ref class MediaSampleEncoder sealed
    {
    public:

        ///<summary>Save an uncompressed video sample to file</summary>
        static Windows::Foundation::IAsyncAction^ SaveToFileAsync(
            _In_ MediaSample^ sample,
            _In_ Windows::Storage::IStorageFile^ file,
            _In_ ContainerFormat container
            );

        ///<summary>Save an uncompressed video sample to stream</summary>
        static Windows::Foundation::IAsyncAction^ SaveToStreamAsync(
            _In_ MediaSample^ sample,
            _In_ Windows::Storage::Streams::IRandomAccessStream^ stream,
            _In_ ContainerFormat container
            );

    private:

        MediaSampleEncoder()
        {
        }
    };

}

