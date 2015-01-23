#include "pch.h"
#include "MediaSample.h"
#include "MediaBuffer2D.h"
#include "ImageDecoder.h"

using namespace concurrency;
using namespace MediaCaptureReader;
using namespace Microsoft::WRL;
using namespace Platform;
using namespace Windows::Foundation;
using namespace Windows::Graphics::Imaging;
using namespace Windows::Media::MediaProperties;
using namespace Windows::Storage;
using namespace Windows::Storage::Streams;

IAsyncOperation<MediaSample2D^>^ ImageDecoder::LoadFromPathAsync(
    Platform::String^ path,
    _In_ MediaSample2DFormat format
    )
{
    return create_async([path, format]()
    {
        return create_task(StorageFile::GetFileFromApplicationUriAsync(ref new WF::Uri(path))).then([format](StorageFile^ file)
        {
            return ImageDecoder::LoadFromFileAsync(file, format);
        });
    });
}

IAsyncOperation<MediaSample2D^>^ ImageDecoder::LoadFromFileAsync(
    _In_ IStorageFile^ file,
    _In_ MediaSample2DFormat format
    )
{
    CHKNULL(file);

    return create_async([file, format]()
    {
        return create_task(file->OpenAsync(FileAccessMode::Read)).then([format](IRandomAccessStream^ stream)
        {
            return ImageDecoder::LoadFromStreamAsync(stream, format);
        });
    });
}

IAsyncOperation<MediaSample2D^>^ ImageDecoder::LoadFromStreamAsync(
    _In_ IRandomAccessStream^ stream,
    _In_ MediaSample2DFormat format
    )
{
    CHKNULL(stream);

    return create_async([stream, format]()
    {
        if (format != MediaSample2DFormat::Nv12)
        {
            CHK(OriginateError(MF_E_UNSUPPORTED_FORMAT));
        }

        ComPtr<IWICImagingFactory> factory;
        CHK(CoCreateInstance(CLSID_WICImagingFactory, nullptr, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&factory)));

        ComPtr<IStream> wrappedStream;
        CHK(CreateStreamOverRandomAccessStream(reinterpret_cast<IUnknown*>(stream), IID_PPV_ARGS(&wrappedStream)));

        ComPtr<IWICBitmapDecoder> decoder;
        CHK(factory->CreateDecoderFromStream(wrappedStream.Get(), nullptr, WICDecodeMetadataCacheOnDemand, &decoder));

        ComPtr<IWICBitmapFrameDecode> frame;
        unsigned int width;
        unsigned int height;
        CHK(decoder->GetFrame(0, &frame));
        CHK(frame->GetSize(&width, &height));

        auto sample = ref new MediaSample2D(MediaSample2DFormat::Nv12, width, height);
        auto buffer = sample->LockBuffer(BufferAccessMode::Write);
        auto planeY = buffer->Planes->GetAt(0);
        auto planeUV = buffer->Planes->GetAt(1);

        WICBitmapPlane planes[2] = {};
        planes[0].Format = GUID_WICPixelFormat8bppY;
        planes[0].cbStride = planeY->Pitch;
        planes[0].cbBufferSize = planeY->Pitch * height;
        planes[0].pbBuffer = GetData(planeY->Buffer);
        planes[1].Format = GUID_WICPixelFormat16bppCbCr;
        planes[1].cbStride = planeUV->Pitch;
        planes[1].cbBufferSize = (planeUV->Pitch * height / 2);
        planes[1].pbBuffer = GetData(planeUV->Buffer);

        CHK(As<IWICPlanarBitmapSourceTransform>(frame)->CopyPixels(
            nullptr,
            width,
            height,
            WICBitmapTransformRotate0,
            WICPlanarOptionsDefault,
            planes,
            2
            ));

        return sample;
    });
}
