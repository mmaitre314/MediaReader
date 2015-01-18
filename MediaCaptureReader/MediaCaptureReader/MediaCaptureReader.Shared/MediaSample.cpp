#include "pch.h"
#include "MediaSample.h"
#include "MediaBuffer2D.h"

using namespace MediaCaptureReader;
using namespace Microsoft::WRL;
using namespace Platform;

MediaSample1D::MediaSample1D(_In_ const MW::ComPtr<IMFSample>& sample)
    : _sample(sample)
{
    assert(sample != nullptr);
}

MediaSample1D::~MediaSample1D()
{
}

MediaSample2D::MediaSample2D(
    _In_ const MW::ComPtr<IMFSample>& sample, 
    _In_ MediaSample2DFormat format, 
    _In_ int width, 
    _In_ int height, 
    _In_opt_ MediaGraphicsDevice^ graphicsDevice /*= nullptr*/
    )
    : _sample(sample)
    , _format(format)
    , _width(width)
    , _height(height)
    , _graphicsDevice(graphicsDevice)
{
    assert(sample != nullptr);
}

MediaSample2D::MediaSample2D(_In_ MediaSample2DFormat format, _In_ int width, _In_ int height)
{
    ComPtr<IMFMediaBuffer> buffer;
    CHK(MFCreate2DMediaBuffer(width, height, GetFourCcFromFormat(format), /*fBottomUp*/false, &buffer));

    // Avoid issues with SinkWriter
    unsigned long length;
    CHK(As<IMF2DBuffer2>(buffer)->GetContiguousLength(&length));
    CHK(buffer->SetCurrentLength(length));

    CHK(MFCreateSample(&_sample));
    CHK(_sample->AddBuffer(buffer.Get()));

    _format = format;
    _width = width;
    _height = height;
}

MediaSample2D::~MediaSample2D()
{
    auto lock = _lock.LockExclusive();
    _sample = nullptr;
}

MediaBuffer2D^ MediaSample2D::LockBuffer(_In_ BufferAccessMode accessMode)
{
    auto lock = _lock.LockExclusive();

    if (_sample == nullptr)
    {
        CHK(OriginateError(RO_E_CLOSED));
    }

    ComPtr<IMFMediaBuffer> buffer;
    CHK(_sample->GetBufferByIndex(0, &buffer));

    // TODO: handle accessMode properly in 1D case

    auto mediaBuffer = ref new MediaBuffer2D(_format, _width, _height);
    mediaBuffer->Initialize(buffer, accessMode, _GetDefaultPitch());
    return mediaBuffer;
}

unsigned int MediaSample2D::_GetDefaultPitch() const
{
    unsigned int size = 0;

    switch (_format)
    {
    case MediaSample2DFormat::Nv12: return _width;
    case MediaSample2DFormat::Bgra8: return 4 * _width;
    case MediaSample2DFormat::Yuy2: return ((2 * _width) + 3) & ~3; // UINT32 aligned
    default: throw ref new Platform::InvalidArgumentException(L"format");
    }
}
