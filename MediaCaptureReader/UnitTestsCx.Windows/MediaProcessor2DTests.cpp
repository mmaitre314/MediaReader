#include "pch.h"

using namespace MediaCaptureReader;
using namespace Microsoft::VisualStudio::CppUnitTestFramework;
using namespace Microsoft::WRL;
using namespace Platform;
using namespace Platform::Collections;
using namespace std;
using namespace Windows::ApplicationModel::Core;
using namespace Windows::Foundation;
using namespace Windows::Graphics::Imaging;
using namespace Windows::Media::Capture;
using namespace Windows::Media::MediaProperties;
using namespace Windows::Storage;
using namespace Windows::Storage::Streams;

TEST_CLASS(MediaProcessor2DTests)
{
public:

    TEST_METHOD(CX_W_MediaProcessor2D_Rotate)
    {
        auto reader = Await(MediaReader::CreateFromPathAsync(L"ms-appx:///car.mp4"));

        Log() << "Reading sample";
        auto result = Await(reader->VideoStream->ReadAsync());
        auto sample = safe_cast<MediaSample2D^>(result->Sample);

        Log() << L"Creating MediaProcessor2D";
        auto processor = ref new MediaProcessor2D();

        auto folder = Await(KnownFolders::PicturesLibrary->CreateFolderAsync(L"MediaCaptureReaderTests", CreationCollisionOption::OpenIfExists));

        Log() << "Rotating and saving sample - 90 degree";
        auto file = Await(folder->CreateFileAsync(L"CX_W_MediaProcessor2D_Rotate_90clockwise.jpg", CreationCollisionOption::ReplaceExisting));
        auto sampleRotated = processor->Rotate(sample, BitmapRotation::Clockwise90Degrees);
        Await(MediaEncoder2D::SaveToFileAsync(sampleRotated, file, ImageCompression::Jpeg));
        Log() << L"Saved " << file->Path->Data();

        Log() << "Rotating and saving sample - 180 degree";
        file = Await(folder->CreateFileAsync(L"CX_W_MediaProcessor2D_Rotate_180clockwise.jpg", CreationCollisionOption::ReplaceExisting));
        sampleRotated = processor->Rotate(sample, BitmapRotation::Clockwise180Degrees);
        Await(MediaEncoder2D::SaveToFileAsync(sampleRotated, file, ImageCompression::Jpeg));
        Log() << L"Saved " << file->Path->Data();

        Log() << "Rotating and saving sample - 270 degree";
        file = Await(folder->CreateFileAsync(L"CX_W_MediaProcessor2D_Rotate_270clockwise.jpg", CreationCollisionOption::ReplaceExisting));
        sampleRotated = processor->Rotate(sample, BitmapRotation::Clockwise270Degrees);
        Await(MediaEncoder2D::SaveToFileAsync(sampleRotated, file, ImageCompression::Jpeg));
        Log() << L"Saved " << file->Path->Data();
    }

    TEST_METHOD(CX_W_MediaProcessor2D_Convert)
    {
        auto reader = Await(MediaReader::CreateFromPathAsync(L"ms-appx:///car.mp4"));

        Log() << "Reading sample";
        auto result = Await(reader->VideoStream->ReadAsync());
        auto sample = safe_cast<MediaSample2D^>(result->Sample);

        Log() << L"Creating MediaProcessor2D";
        auto processor = ref new MediaProcessor2D();

        auto folder = Await(KnownFolders::PicturesLibrary->CreateFolderAsync(L"MediaCaptureReaderTests", CreationCollisionOption::OpenIfExists));

        Log() << "Converting sample - format";
        auto file = Await(folder->CreateFileAsync(L"CX_W_MediaProcessor2D_Convert_Format.jpg", CreationCollisionOption::ReplaceExisting));
        Assert::IsTrue(sample->Format == MediaSample2DFormat::Nv12);
        auto sampleConverted = processor->Convert(sample, MediaSample2DFormat::Bgra8, sample->Width, sample->Height);
        Await(MediaEncoder2D::SaveToFileAsync(sampleConverted, file, ImageCompression::Jpeg));
        Log() << L"Saved " << file->Path->Data();

        Log() << "Converting sample - scale";
        file = Await(folder->CreateFileAsync(L"CX_W_MediaProcessor2D_Convert_Scale.jpg", CreationCollisionOption::ReplaceExisting));
        sampleConverted = processor->Convert(sample, sample->Format, sample->Width / 2, sample->Height / 2);
        Await(MediaEncoder2D::SaveToFileAsync(sampleConverted, file, ImageCompression::Jpeg));
        Log() << L"Saved " << file->Path->Data();

        Log() << "Converting sample - scale width";
        file = Await(folder->CreateFileAsync(L"CX_W_MediaProcessor2D_Convert_ScaleWidth.jpg", CreationCollisionOption::ReplaceExisting));
        sampleConverted = processor->Convert(sample, sample->Format, sample->Width * 2, sample->Height);
        Await(MediaEncoder2D::SaveToFileAsync(sampleConverted, file, ImageCompression::Jpeg));
        Log() << L"Saved " << file->Path->Data();
    }
};