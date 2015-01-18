#include "pch.h"
#include "WinRTBuffer.h"
#include "MediaSample.h"
#include "MediaBuffer2D.h"

using namespace MediaCaptureReader;
using namespace Microsoft::WRL;
using namespace Platform;
using namespace Platform::Collections;
using namespace Windows::Storage::Streams;

MediaBuffer2D::MediaBuffer2D(
    _In_ MediaSample2DFormat format,
    _In_ int width,
    _In_ int height
    )
    : _buffer(nullptr)
    , _capacity(0)
    , _pitch(0)
    , _format(format)
    , _width(width)
    , _height(height)
{
}

void MediaBuffer2D::Initialize(
    _In_ const ComPtr<IMFMediaBuffer>& buffer, 
    _In_ BufferAccessMode accessMode, 
    unsigned int defaultPitch
    )
{
    if (SUCCEEDED(buffer.As(&_buffer2D)))
    {
        MF2DBuffer_LockFlags lockFlags = _GetLockFlagsFromAccessMode(accessMode);

        unsigned char *scanline0 = nullptr;
        long pitch;
        CHK(_buffer2D->Lock2DSize(lockFlags, &scanline0, &pitch, &_buffer, &_capacity));

        if (pitch <= 0)
        {
            CHK(OriginateError(E_INVALIDARG, L"Negative pitch"));
        }

        _pitch = static_cast<unsigned int>(pitch);
    }
    else
    {
        // When inserted in MediaElement effects may get 1D buffers (maybe in other cases too)
        // so support fallback to 1D buffers here

        unsigned long capacity;
        unsigned long length;
        CHK(buffer->Lock(&_buffer, &capacity, &length));

        _buffer1D = buffer;
        _capacity = capacity;
        _pitch = defaultPitch;
    }

    auto planes = ref new Vector<MediaBufferPlane^>();

    switch (_format)
    {
    case MediaSample2DFormat::Bgra8:
    case MediaSample2DFormat::Yuy2:
        planes->Append(ref new MediaBufferPlane(_buffer, _height * _pitch, _pitch));
        break;

    case MediaSample2DFormat::Nv12:
        planes->Append(ref new MediaBufferPlane(_buffer, _height * _pitch, _pitch)); // Y
        planes->Append(ref new MediaBufferPlane(_buffer + _height * _pitch, (_height * _pitch) / 2, _pitch)); // UV
        break;

    default: CHK(OriginateError(E_INVALIDARG, L"format"));
    }

    _planes = planes->GetView();
}

MediaBuffer2D::~MediaBuffer2D()
{
    auto lock = _lock.LockExclusive();
    _Close();
}

void MediaBuffer2D::_Close()
{
    if ((_buffer2D != nullptr) && (_buffer != nullptr))
    {
        (void)_buffer2D->Unlock2D();
    }
    if (_buffer1D != nullptr)
    {
        (void)_buffer1D->Unlock();
    }
    if (_planes != nullptr)
    {
        for (auto plane : _planes)
        {
            plane->Close();
        }
    }

    _buffer2D = nullptr;
    _buffer1D = nullptr;
    _buffer = nullptr;
    _capacity = 0;
    _pitch = 0;
    _format = MediaSample2DFormat::Unknown;
    _width = 0;
    _height = 0;
}

MediaBufferPlane::MediaBufferPlane(
    _In_reads_(_Inexpressible_) unsigned char *buffer,
    _In_ unsigned long capacity,
    _In_ unsigned int pitch
    )
    : _pitch(pitch)
{
    _buffer = Make<WinRTBuffer>(buffer, capacity);
    CHKOOM(_buffer);
}

MediaBufferPlane::~MediaBufferPlane()
{
}

IBuffer^ MediaBufferPlane::Buffer::get()
{
    auto lock = _lock.LockShared();
    return reinterpret_cast<WSS::IBuffer^>(static_cast<AWSS::IBuffer*>(_buffer.Get()));
}

uint32 MediaBufferPlane::Pitch::get()
{
    auto lock = _lock.LockShared();
    return _pitch;
}

void MediaBufferPlane::Close()
{
    auto lock = _lock.LockExclusive();

    if (_buffer == nullptr)
    {
        return;
    }

    _buffer->Close();
    _buffer = nullptr;
    _pitch = 0;
}
