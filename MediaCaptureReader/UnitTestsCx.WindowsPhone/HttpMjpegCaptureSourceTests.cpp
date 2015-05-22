#include "pch.h"

using namespace MediaCaptureReader;
using namespace Microsoft::VisualStudio::CppUnitTestFramework;
using namespace Platform;
using namespace Windows::Foundation;
using namespace Windows::Media::Capture;
using namespace Windows::Media::MediaProperties;
using namespace Windows::Storage;
using namespace Windows::Storage::Streams;

TEST_CLASS(HttpMjpegCaptureSourceTests)
{
public:

    TEST_METHOD(CX_WP_HttpMjpegCaptureSourceTests_Basic)
    {
        Log() << L"Creating HttpMjpegCaptureSource";
        auto source = Await(HttpMjpegCaptureSource::CreateFromUriAsync("http://216.123.238.208/axis-cgi/mjpg/video.cgi?camera&resolution=640x480"));

        Log() << L"Creating MediaReader";
        auto reader = Await(MediaReader::CreateFromMediaSourceAsync(source->Source));

        Log() << L"Reading sample";
        auto result = Await(reader->VideoStream->ReadAsync());
        auto sample = safe_cast<MediaSample2D^>(result->Sample);

        Assert::AreEqual(MediaSample2DFormat::Nv12, sample->Format);
    }
};
