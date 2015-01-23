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

TEST_CLASS(ImageDecoderTests)
{
public:

    TEST_METHOD(CX_W_ImageDecoder_Basic)
    {
        Log() << "Reading sample";
        auto sample = Await(ImageDecoder::LoadFromPathAsync(L"ms-appx:///car.jpg", MediaSample2DFormat::Nv12));
        Assert::AreEqual(320, sample->Width);
        Assert::AreEqual(240, sample->Height);
        Assert::AreEqual(MediaSample2DFormat::Nv12, sample->Format);
        Assert::AreEqual(0ll, sample->Duration.Duration);
        Assert::AreEqual(0ll, sample->Timestamp.Duration);
        Assert::IsNull(sample->GraphicsDevice);
        
        auto folder = Await(KnownFolders::PicturesLibrary->CreateFolderAsync(L"MediaCaptureReaderTests", CreationCollisionOption::OpenIfExists));

        Log() << "Saving sample";
        auto file = Await(folder->CreateFileAsync(L"CX_W_ImageDecoder_Basic.jpg", CreationCollisionOption::ReplaceExisting));
        Await(ImageEncoder::SaveToFileAsync(sample, file, ImageCompression::Jpeg));
        Log() << L"Saved " << file->Path->Data();
    }
};