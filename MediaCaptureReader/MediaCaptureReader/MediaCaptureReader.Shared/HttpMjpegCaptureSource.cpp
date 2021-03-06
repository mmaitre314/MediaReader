﻿#include "pch.h"
#include "HttpMjpegCaptureSource.h"

using namespace concurrency;
using namespace MediaCaptureReader;
using namespace Microsoft::WRL;
using namespace Platform;
using namespace std;
using namespace Windows::Foundation;
using namespace Windows::Graphics::Imaging;
using namespace Windows::Media::Core;
using namespace Windows::Media::MediaProperties;
using namespace Windows::Storage::Streams;
using namespace Windows::System::Threading;
using namespace Windows::Web::Http;

HttpMjpegCaptureSource::HttpMjpegCaptureSource()
    : _frameRate(0)
    , _streamReadBuffer(ref new Buffer(1024 * 1024))
    , _started(false)
    , _closed(false)
    , _timeOffset(0)
    , _discontinuity(false)
{
    
	#ifdef NTRACELOG
		Logger.HttpMjpegCaptureSource_LifeTimeStart((void*)this);
	#endif

    CHK(CoCreateInstance(CLSID_WICImagingFactory, nullptr, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&_wicFactory)));
}

HttpMjpegCaptureSource::~HttpMjpegCaptureSource()
{
    auto lock = _lock.LockExclusive();

    _started = false;
    _closed = true;

    queue<MediaStreamSample^>().swap(_samples);
    queue<MediaStreamSourceSampleRequest^>().swap(_sampleRequests);
    queue<MediaStreamSourceSampleRequestDeferral^>().swap(_sampleRequestDeferrals);

    delete _stream;
    _stream = nullptr;

    _source = nullptr;
    _streamReadBuffer = nullptr;
    _message.Clear();

	#ifdef NTRACELOG
		Logger.HttpMjpegCaptureSource_LifeTimeStop((void*)this);
	#endif
}

IAsyncOperation<HttpMjpegCaptureSource^>^ HttpMjpegCaptureSource::CreateFromUriAsync(_In_ Uri^ uri)
{
	#ifdef NTRACELOG
		Trace("Creating HttpMjpegCaptureSource");
	#endif

    CHKNULL(uri);

    return create_async([uri]()
    {
        auto httpClient = ref new HttpClient();
        auto source = ref new HttpMjpegCaptureSource();

        return create_task(httpClient->GetAsync(uri, HttpCompletionOption::ResponseHeadersRead)).then([source](HttpResponseMessage^ httpResponse)
        {
            httpResponse->EnsureSuccessStatusCode();

			#ifdef NTRACELOG
				Trace("HTTP response headers:");
				for (auto header : httpResponse->Headers)
				{
					Trace("  %S: %S", header->Key->Data(), header->Value->Data());
				}

				Trace("HTTP response content headers:");
				for (auto header : httpResponse->Content->Headers)
				{
					Trace("  %S: %S", header->Key->Data(), header->Value->Data());
				}
			#endif

            auto headers = httpResponse->Content->Headers;
            if (headers->ContentType->MediaType != "multipart/x-mixed-replace")
            {
                throw ref new Platform::FailureException(L"Invalid ContentType: expected 'multipart/x-mixed-replace', received '" + headers->ContentType->MediaType + L"'");
            }

            // Get the message boundary
            bool found = false;
            for (auto param : headers->ContentType->Parameters)
            {
                if (param->Name == "boundary")
                {
                    source->_message.SetBoundary(param->Value);
                    found = true;
                    break;
                }
            }
            if (!found)
            {
                throw ref new Platform::FailureException(L"HTTP multipart boundary not found");
            }

            if (httpResponse->Headers->HasKey(L"X-FrameRate"))
            {
                source->_frameRate = stoi(httpResponse->Headers->Lookup("X-FrameRate")->Data());
            }
			#ifdef NTRACELOG
				Trace("Framerate: %i", source->_frameRate);
			#endif
            return httpResponse->Content->ReadAsInputStreamAsync();
        }).then([source](IInputStream^ stream)
        {
            source->_stream = stream;
            return source->_InitializeAsync();
        }).then([source]()
        {
			#ifdef NTRACELOG
				Trace("HttpMjpegCaptureSource created");
			#endif
            return source;
        });
    });
}

IAsyncOperation<HttpMjpegCaptureSource^>^ HttpMjpegCaptureSource::CreateFromStreamAsync(_In_ WSS::IInputStream^ stream, _In_ String^ boundary)
{
	#ifdef NTRACELOG
		Trace("Creating HttpMjpegCaptureSource");
	#endif

    CHKNULL(stream);
    CHKNULL(boundary);

    auto source = ref new HttpMjpegCaptureSource();

    return create_async([stream, boundary, source]()
    {
        source->_stream = stream;
        source->_message.SetBoundary(boundary);

        return create_task(source->_InitializeAsync()).then([source]()
        {
			#ifdef NTRACELOG
				Trace("HttpMjpegCaptureSource created");
			#endif
            return source;
        });
    });
}

task<void> HttpMjpegCaptureSource::_InitializeAsync()
{
    // Read and decode a frame to get the video width/height
    return _ReadSingleFrameAsync().then([this](IBuffer^ buffer)
    {
        // Get the video width/height
        ComPtr<IWICStream> stream;
        ComPtr<IWICBitmapDecoder> decoder;
        ComPtr<IWICBitmapFrameDecode> frame;
        unsigned int width;
        unsigned int height;
        CHK(_wicFactory->CreateStream(&stream));
        CHK(stream->InitializeFromMemory(GetData(buffer), buffer->Length));
        CHK(_wicFactory->CreateDecoderFromStream(stream.Get(), &GUID_ContainerFormatJpeg, WICDecodeMetadataCacheOnDemand, &decoder));
        CHK(decoder->GetFrame(0, &frame));
        CHK(frame->GetSize(&width, &height));

        // Create the video props
        auto encodingProps = ref new VideoEncodingProperties();
        encodingProps->Subtype = _decodeMJPEG ? MediaEncodingSubtypes::Nv12 : MediaEncodingSubtypes::Mjpg;
        encodingProps->Width = width;
        encodingProps->Height = height;
        if (_frameRate != 0)
        {
            encodingProps->FrameRate->Numerator = _frameRate;
            encodingProps->FrameRate->Denominator = 1;
        }

		#ifdef NTRACELOG
				Trace("Source output subtype: %S", encodingProps->Subtype->Data());
		#endif

        auto source = ref new MediaStreamSource(ref new VideoStreamDescriptor(encodingProps));
        source->CanSeek = false;

        source->Starting += ref new TypedEventHandler<MediaStreamSource^, MediaStreamSourceStartingEventArgs^>(
            [this](MediaStreamSource^, MediaStreamSourceStartingEventArgs^)
        {
			#ifdef NTRACELOG
				Trace("Starting event received");
			#endif
            auto lock = _lock.LockExclusive();
            _started = true;
        });

        source->SampleRequested += ref new TypedEventHandler<MediaStreamSource^, MediaStreamSourceSampleRequestedEventArgs^>(
            [this](MediaStreamSource^, MediaStreamSourceSampleRequestedEventArgs^ e)
        {
            auto lock = _lock.LockExclusive();
            if (!_closed)
            {
				#ifdef NTRACELOG
					Trace("SampleRequested event received");
				#endif
                _QueueRequest(e->Request);
            }
        });

        source->Closed += ref new TypedEventHandler<MediaStreamSource^, MediaStreamSourceClosedEventArgs^>(
            [this](MediaStreamSource^, MediaStreamSourceClosedEventArgs^)
        {
			#ifdef NTRACELOG
				Trace("Closed event received");
			#endif
            delete this;
        });

        // Start streaming
        _ReadFramesAsync();

        _source = source; // This creates a circular reference which needs to be explicitly broken
    });
}

void HttpMjpegCaptureSource::_ReadFramesAsync()
{
    _ReadSingleFrameAsync().then([this](IBuffer^ buffer)
    {
        auto lock = _lock.LockExclusive();

        if (_started)
        {
            MFTIME time = MFGetSystemTime(); // TODO: handle the X-TimeStamp header

            // Rebase timestamps to force starting at 0
            if (_timeOffset == 0)
            {
                _timeOffset = time;
            }
            time -= _timeOffset;

            // Decode the JPEG buffer to NV12 if the platform does not have an MJPEG decoder
            if (_decodeMJPEG)
            {
				#ifdef NTRACELOG
					Logger.HttpMjpegCaptureSource_DecodeMjpegStart((void*)this);
				#endif

                ComPtr<IWICStream> stream;
                ComPtr<IWICBitmapDecoder> decoder;
                ComPtr<IWICBitmapFrameDecode> frame;
                unsigned int width;
                unsigned int height;
                CHK(_wicFactory->CreateStream(&stream));
                CHK(stream->InitializeFromMemory(GetData(buffer), buffer->Length));
                CHK(_wicFactory->CreateDecoderFromStream(stream.Get(), &GUID_ContainerFormatJpeg, WICDecodeMetadataCacheOnDemand, &decoder));
                CHK(decoder->GetFrame(0, &frame));
                CHK(frame->GetSize(&width, &height));

                buffer = ref new Buffer((3 * width * height) / 2);
                buffer->Length = buffer->Capacity;

                WICBitmapPlane planes[2] = {};
                planes[0].Format = GUID_WICPixelFormat8bppY;
                planes[0].pbBuffer = GetData(buffer);
                planes[0].cbStride = width;
                planes[0].cbBufferSize = width * height;
                planes[1].Format = GUID_WICPixelFormat16bppCbCr;
                planes[1].pbBuffer = planes[0].pbBuffer + planes[0].cbBufferSize;
                planes[1].cbStride = width;
                planes[1].cbBufferSize = width * height / 2;
                CHK(As<IWICPlanarBitmapSourceTransform>(frame)->CopyPixels(
                    nullptr,
                    width,
                    height,
                    WICBitmapTransformRotate0,
                    WICPlanarOptionsDefault,
                    planes,
                    2
                    ));

				#ifdef NTRACELOG
					Logger.HttpMjpegCaptureSource_DecodeMjpegStop((void*)this);
				#endif
            }

            auto sample = MediaStreamSample::CreateFromBuffer(buffer, { time });
            sample->KeyFrame = true;

            _QueueSample(sample);
        }

        if (!_closed)
        {
            _ReadFramesAsync();
        }
    }).then([this](task<void> t)
    {
        try
        {
            t.get();
        }
        catch (Exception^ e)
        {
            auto lock = _lock.LockExclusive();
			#ifdef NTRACELOG
				TraceError("Error: 0x%08X %S", e->HResult, e->Message)
			#endif
            if (_source != nullptr)
            {
                _source->NotifyError(MediaStreamSourceErrorStatus::Other);
            }
        }
    });
}

task<IBuffer^> HttpMjpegCaptureSource::_ReadSingleFrameAsync()
{
    IBuffer^ buffer = _message.GetPart();
    if (buffer != nullptr)
    {
        return task_from_result(static_cast<IBuffer^>(buffer));
    }

    // If not found, request more data and try again
    return create_task([this]()
    {
        // Call ReadAsync() on a different thread to avoid deadlocks

        auto lock = _lock.LockExclusive();
        if (_closed)
        {
            throw ref new OperationCanceledException();
        }

        _streamReadBuffer->Length = 0;
        return _stream->ReadAsync(_streamReadBuffer, _streamReadBuffer->Capacity, InputStreamOptions::Partial);
    }).then([this](IBuffer^ readBuffer)
    {
        auto lock = _lock.LockExclusive();
        if (_closed)
        {
            throw ref new OperationCanceledException();
        }

		#ifdef NTRACELOG
			Trace("Read %iB from stream", readBuffer->Length);
		#endif

        if (readBuffer->Length > 0)
        {
            _message.Append(readBuffer);
        }
        else
        {
            // Simulate infinite streams from files
            auto raStream = dynamic_cast<IRandomAccessStream^>(_stream);
            if ((raStream != nullptr) && (raStream->Size > 0) && (raStream->Position >= raStream->Size))
            {
				#ifdef NTRACELOG
					Trace("Looping stream");
				#endif
                raStream->Seek(0);
            }
        }
        
        return _ReadSingleFrameAsync();
    });
}

void HttpMjpegCaptureSource::_QueueRequest(_In_ MediaStreamSourceSampleRequest ^request)
{
	#ifdef NTRACELOG
		Trace("Queuing request");
	#endif
    _sampleRequests.push(request);
    _sampleRequestDeferrals.push(request->GetDeferral());
    _ProcessQueue();
}

void HttpMjpegCaptureSource::_QueueSample(_In_ MediaStreamSample^ sample)
{
	#ifdef NTRACELOG
		Trace("Queuing sample: time %I64ihns", sample->Timestamp.Duration);
	#endif

    if (_samples.size() >= 2)
    {
		#ifdef NTRACELOG
			Trace("Dropping sample: time %I64ihns", _samples.front()->Timestamp.Duration);
		#endif
        _discontinuity = true;
        _samples.pop();
    }

    _samples.push(sample);
    _ProcessQueue();
}

void HttpMjpegCaptureSource::_ProcessQueue()
{
    while ((_sampleRequests.size() > 0) && (_samples.size() > 0))
    {

        auto request = _sampleRequests.front();
        auto deferral = _sampleRequestDeferrals.front();
        request->Sample = _samples.front();
        request->Sample->Discontinuous = _discontinuity;
        _discontinuity = false;

		#ifdef NTRACELOG
			Trace("Sending sample: time %I64ihns", _samples.front()->Timestamp.Duration);
		#endif

        deferral->Complete();

        _sampleRequests.pop();
        _sampleRequestDeferrals.pop();
        _samples.pop();
    }
}
