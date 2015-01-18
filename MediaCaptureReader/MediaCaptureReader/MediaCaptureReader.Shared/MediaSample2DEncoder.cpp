#include "pch.h"
#include "MediaSample2DEncoder.h"
#include "MediaSample.h"

using namespace concurrency;
using namespace MediaCaptureReader;
using namespace Microsoft::WRL;
using namespace Platform;
using namespace Windows::Foundation;
using namespace Windows::Graphics::Imaging;
using namespace Windows::Media::MediaProperties;
using namespace Windows::Storage;
using namespace Windows::Storage::Streams;

IAsyncAction^ MediaSample2DEncoder::SaveToFileAsync(
    _In_ MediaSample2D^ sample,
    _In_ Windows::Storage::IStorageFile^ file,
    _In_ ImageCompression compression
    )
{
    CHKNULL(file);
    CHKNULL(sample);

    return create_async([file, sample, compression]()
    {
        return create_task(file->OpenAsync(FileAccessMode::ReadWrite)).then([file, sample, compression](IRandomAccessStream^ stream)
        {
            return MediaSample2DEncoder::SaveToStreamAsync(sample, stream, compression);
        });
    });
}

IAsyncAction^ MediaSample2DEncoder::SaveToStreamAsync(
    _In_ MediaSample2D^ sample,
    _In_ Windows::Storage::Streams::IRandomAccessStream^ stream,
    _In_ ImageCompression compression
    )
{
    CHKNULL(stream);
    CHKNULL(sample);

    if ((sample->Format != MediaSample2DFormat::Bgra8) && (sample->Format != MediaSample2DFormat::Nv12))
    {
        CHK(OriginateError(MF_E_UNSUPPORTED_FORMAT));
    }
    if (compression != ImageCompression::Jpeg)
    {
        CHK(OriginateError(MF_E_UNSUPPORTED_FORMAT));
    }

    return create_async([stream, sample, compression]()
    {
        ComPtr<IMFSample> sampleMf = sample->GetSample();
        ComPtr<IMFMediaBuffer> buffer;
        CHK(sampleMf->GetBufferByIndex(0, &buffer));
        int width = sample->Width;
        int height = sample->Height;

        // For Bgra8 use regular BitmapEncoder
        if (sample->Format == MediaSample2DFormat::Bgra8)
        {
            return create_task(BitmapEncoder::CreateAsync(BitmapEncoder::JpegEncoderId, stream)).then([buffer, width, height](BitmapEncoder^ encoder)
            {
                unsigned char *data = nullptr;
                unsigned long length;
                CHK(buffer->Lock(&data, &length, nullptr));
                OnScopeExit([buffer]()
                {
                    (void)buffer->Unlock();
                });

                encoder->SetPixelData(
                    BitmapPixelFormat::Bgra8, 
                    BitmapAlphaMode::Ignore, 
                    width, 
                    height, 
                    96., 
                    96., 
                    ArrayReference<uint8>(data, length)
                    );

                return encoder->FlushAsync();
            });
        }

        auto buffer2D = As<IMF2DBuffer2>(buffer);
        unsigned char *data = nullptr;
        unsigned char *ignore = nullptr;
        long stride;
        unsigned long length;
        CHK(buffer2D->Lock2DSize(MF2DBuffer_LockFlags_Read, &ignore, &stride, &data, &length));
        OnScopeExit([buffer2D]()
        {
            (void)buffer2D->Unlock2D();
        });

        if (stride <= 0)
        {
            CHK(OriginateError(E_INVALIDARG, L"Negative stride"));
        }

        if ((int)length < (height * stride * 3) / 2)
        {
            CHK(OriginateError(MF_E_BUFFERTOOSMALL));
        }

        WICBitmapPlane planes[2] = {};
        planes[0].Format = GUID_WICPixelFormat8bppY;
        planes[0].cbStride = stride;
        planes[0].cbBufferSize = height * stride;
        planes[0].pbBuffer = data;
        planes[1].Format = GUID_WICPixelFormat16bppCbCr;
        planes[1].cbStride = stride;
        planes[1].cbBufferSize = (height * stride) / 2;
        planes[1].pbBuffer = data + height * stride;

        ComPtr<IWICBitmapEncoder> encoder;
        ComPtr<IWICImagingFactory> factory;
        CHK(CoCreateInstance(CLSID_WICImagingFactory, nullptr, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&factory)));
        CHK(factory->CreateEncoder(GUID_ContainerFormatJpeg, nullptr, &encoder));

        ComPtr<IStream> wrappedStream;
        CHK(CreateStreamOverRandomAccessStream(
            reinterpret_cast<IUnknown*>(stream),
            IID_PPV_ARGS(&wrappedStream)
            ));

        CHK(encoder->Initialize(wrappedStream.Get(), WICBitmapEncoderNoCache));

        ComPtr<IWICBitmapFrameEncode> frame;
        ComPtr<IPropertyBag2> propertyBag;
        WICPixelFormatGUID wicFormat = GUID_WICPixelFormat24bppBGR;
        CHK(encoder->CreateNewFrame(&frame, nullptr));
        CHK(frame->Initialize(nullptr));
        CHK(frame->SetPixelFormat(&wicFormat));
        CHK(frame->SetSize(width, height));

        CHK(As<IWICPlanarBitmapFrameEncode>(frame)->WritePixels(height, planes, 2));
        CHK(frame->Commit());
        CHK(encoder->Commit());

        return task_from_result();
    });
}
