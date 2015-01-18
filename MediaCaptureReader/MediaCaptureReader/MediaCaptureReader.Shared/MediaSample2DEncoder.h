#pragma once

namespace MediaCaptureReader
{
    ref class MediaSample2D;

    public enum class ImageCompression
    {
        Jpeg = 0
    };

    public ref class MediaSample2DEncoder sealed
    {
    public:

        ///<summary>Save an uncompressed video sample to file</summary>
        static Windows::Foundation::IAsyncAction^ SaveToFileAsync(
            _In_ MediaSample2D^ sample,
            _In_ Windows::Storage::IStorageFile^ file,
            _In_ ImageCompression compression
            );

        ///<summary>Save an uncompressed video sample to stream</summary>
        static Windows::Foundation::IAsyncAction^ SaveToStreamAsync(
            _In_ MediaSample2D^ sample,
            _In_ Windows::Storage::Streams::IRandomAccessStream^ stream,
            _In_ ImageCompression compression
            );

    private:

        MediaSample2DEncoder()
        {
        }
    };

}

