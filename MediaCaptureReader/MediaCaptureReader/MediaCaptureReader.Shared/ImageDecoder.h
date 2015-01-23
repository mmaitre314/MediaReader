#pragma once

namespace MediaCaptureReader
{
    ref class MediaSample2D;

    public ref class ImageDecoder sealed
    {
    public:

        /// <summary>Read an audio or video file. Use "ms-appx:///" to load a file from the app package and "ms-appdata:///" to load a file from the app state.</summary>
        static WF::IAsyncOperation<MediaSample2D^>^ LoadFromPathAsync(
            Platform::String^ path,
            _In_ MediaSample2DFormat format
            );

        ///<summary>Loads a 2D media sample from file</summary>
        static WF::IAsyncOperation<MediaSample2D^>^ LoadFromFileAsync(
            _In_ WS::IStorageFile^ file,
            _In_ MediaSample2DFormat format
            );

        ///<summary>Loads a 2D media sample from stream</summary>
        static WF::IAsyncOperation<MediaSample2D^>^ LoadFromStreamAsync(
            _In_ WSS::IRandomAccessStream^ stream,
            _In_ MediaSample2DFormat format
            );

    private:

        ImageDecoder()
        {
        }
    };

}

