#include "pch.h"

using namespace MediaCaptureReader;
using namespace Microsoft::VisualStudio::CppUnitTestFramework;
using namespace Windows::Foundation;
using namespace Windows::Media::Capture;
using namespace Windows::Media::MediaProperties;
using namespace Windows::Storage;
using namespace Windows::Storage::Streams;

TEST_CLASS(HttpMjpegCaptureSourceTests)
{
public:

    TEST_METHOD(CX_W_HttpMjpegCaptureSourceTests_Basic)
    {
        Log() << L"Creating HttpMjpegCaptureSource";
        auto file = Await(StorageFile::GetFileFromApplicationUriAsync(ref new Uri(L"ms-appx:///video.cvmpilj.mjpg")));
        IInputStream^ stream = Await(file->OpenSequentialReadAsync());
        auto source = Await(HttpMjpegCaptureSource::CreateFromStreamAsync(stream, L"myboundary"));

        Log() << L"Creating MediaReader";
        auto reader = Await(MediaReader::CreateFromMediaSourceAsync(source->Source));

        Log() << L"Reading sample";
        auto result = Await(reader->VideoStream->ReadAsync());
        auto sample = safe_cast<MediaSample2D^>(result->Sample);

        Assert::AreEqual(MediaSample2DFormat::Nv12, sample->Format);
    }
};
