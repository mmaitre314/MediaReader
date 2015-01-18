#pragma once

namespace MediaCaptureReader
{
    class WinRTBuffer;

    public ref class MediaBufferPlane sealed
    {

    public:

        property WSS::IBuffer^ Buffer { WSS::IBuffer^ get(); }
        property uint32 Pitch { uint32 get(); }

    internal:

        MediaBufferPlane(
            _In_reads_(_Inexpressible_) unsigned char *buffer,
            _In_ unsigned long capacity,
            _In_ unsigned int pitch
        );

        void Close();

    private:

        ~MediaBufferPlane();

        MW::ComPtr<WinRTBuffer> _buffer;
        unsigned int _pitch;

        MWW::SRWLock _lock;
    };

    public ref class MediaBuffer2D sealed
    {
    public:

        property MediaSample2DFormat Format { MediaSample2DFormat get() { return _format; } }
        property int Width { int get() { return _width; } }
        property int Height { int get() { return _height; } }

        ///<summary>List of bitmap planes (one for Bgra8 and Yuy2, two for Nv12, three for Yv12)</summary>
        property WFC::IVectorView<MediaBufferPlane^>^ Planes { WFC::IVectorView<MediaBufferPlane^>^ get() { return _planes; } }

        // IClosable
        virtual ~MediaBuffer2D();

    internal:

        MediaBuffer2D(
            _In_ MediaSample2DFormat format,
            _In_ int width,
            _In_ int height
            );

        // Separate initialization from constructor so that the destructor gets called in case of exception
        void Initialize(_In_ const MW::ComPtr<IMFMediaBuffer>& buffer, _In_ BufferAccessMode accessMode, unsigned int defaultPitch);

    private:

        static MF2DBuffer_LockFlags _GetLockFlagsFromAccessMode(_In_ BufferAccessMode accessMode)
        {
            switch (accessMode)
            {
            case BufferAccessMode::Read: return MF2DBuffer_LockFlags_Read;
            case BufferAccessMode::Write: return MF2DBuffer_LockFlags_Write;
            case BufferAccessMode::ReadWrite: return MF2DBuffer_LockFlags_ReadWrite;
            default: throw ref new Platform::InvalidArgumentException(L"accessMode");
            }
        }

        void _Close();

        WFC::IVectorView<MediaBufferPlane^>^ _planes;

        MW::ComPtr<IMF2DBuffer2> _buffer2D;
        MW::ComPtr<IMFMediaBuffer> _buffer1D;
        MediaSample2DFormat _format;
        int _width;
        int _height;
        unsigned char *_buffer;
        unsigned long _capacity;
        unsigned int _pitch;

        MWW::SRWLock _lock;
    };

}
