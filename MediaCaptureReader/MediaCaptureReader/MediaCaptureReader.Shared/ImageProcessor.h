#pragma once

namespace MediaCaptureReader
{
    public ref class ImageProcessor sealed
    {
    public:

        ImageProcessor();

        MediaSample2D^ Convert(
            _In_ MediaSample2D^ sample,
            _In_ MediaSample2DFormat format,
            _In_ int width,
            _In_ int height
            );

        MediaSample2D^ Rotate(
            _In_ MediaSample2D^ sample,
            _In_ WGI::BitmapRotation rotation
            );

        // IClosable
        virtual ~ImageProcessor();

    private:

        void _CreateVideoProcessor();
        void _InitializeVideoProcessor();
        MediaSample2D^ _Process(_In_ MediaSample2D^ sample) const;

        MediaSample2DFormat _outputFormat;
        unsigned int _outputWidth;
        unsigned int _outputHeight;

        MediaSample2DFormat _inputFormat;
        unsigned int _inputWidth;
        unsigned int _inputHeight;

        WGI::BitmapRotation _rotation;

        MFT_OUTPUT_STREAM_INFO _outputStreamInfo;

        MediaGraphicsDevice^ _graphicsDevice;
        bool _usingGraphicsDevice;

        MW::ComPtr<IMFTransform> _processor;
        MWW::SRWLock _lock;
        AutoMF _mf;
    };
}
