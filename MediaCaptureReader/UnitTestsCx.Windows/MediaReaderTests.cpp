#include "pch.h"

using namespace Lumia::Imaging;
using namespace Lumia::Imaging::Artistic;
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
using namespace Windows::UI::Core;
using namespace Windows::UI::Xaml::Controls;
using namespace Windows::UI::Xaml::Media::Imaging;

TEST_CLASS(MediaReaderTests)
{
public:

    TEST_METHOD(CX_W_MediaReader_FromFile_Basics)
    {
        auto reader = Await(MediaReader::CreateFromPathAsync(L"ms-appx:///car.mp4"));

        // Test MediaReader state
        Assert::IsTrue(reader->CanSeek);
        Assert::AreEqual(31578911i64, reader->Duration.Duration);
        Assert::IsNotNull(reader->VideoStream);
        Assert::IsNotNull(reader->AudioStream);
        Assert::AreEqual(2u, reader->AllStreams->Size);

        //// TODO
        //// Test access to native IMFSourceReader
        //ComPtr<IMFSourceReaderEx> sourceReader;
        //CHK(GetNativeService<IMediaReaderNative>(reader)->GetSourceReader(&sourceReader));
        //Assert::IsNotNull(sourceReader.Get());

        // Read the first video frame
        Log() << "Reading sample";
        auto result = Await(reader->VideoStream->ReadAsync());

        // Save it as JPEG
        Log() << "Saving sample";
        auto folder = Await(KnownFolders::PicturesLibrary->CreateFolderAsync(L"MediaCaptureReaderTests", CreationCollisionOption::OpenIfExists));
        auto file = Await(folder->CreateFileAsync(L"CX_W_MediaReader_TestBasics.jpg", CreationCollisionOption::ReplaceExisting));
        Await(ImageEncoder::SaveToFileAsync(safe_cast<MediaSample2D^>(result->Sample), file, ImageCompression::Jpeg));
        Log() << L"Saved " << file->Path->Data();
    }

    TEST_METHOD(CX_W_MediaReader_FromFile_ReadVideoUntilEndOfFile)
    {
        auto reader = Await(MediaReader::CreateFromPathAsync(L"ms-appx:///car.mp4", AudioInitialization::Deselected, VideoInitialization::Nv12));
        Assert::IsNotNull(reader->VideoStream);
        Assert::IsNotNull(reader->AudioStream);
        Assert::AreEqual(2u, reader->AllStreams->Size);
        Assert::IsTrue(reader->VideoStream->IsSelected);
        Assert::IsFalse(reader->AudioStream->IsSelected);

        while (true)
        {
            auto result = Await(reader->VideoStream->ReadAsync());
            if (result->EndOfStream)
            {
                Log() << L"EndOfStream reached";
                break;
            }
            if (result->Error)
            {
                Log() << L"Error received 0x" << hex << uppercase << setw(8) << setfill(L'0') << result->ErrorCode << L" " << result->ErrorMessage->Data();
                Assert::Fail();
            }
            Assert::IsNotNull(result->Sample);
        }
    }

    TEST_METHOD(CX_W_MediaReader_FromFile_ReadAudioUntilEndOfFile)
    {
        auto reader = Await(MediaReader::CreateFromPathAsync(L"ms-appx:///car.mp4", AudioInitialization::Pcm, VideoInitialization::Deselected));
        Assert::IsNotNull(reader->VideoStream);
        Assert::IsNotNull(reader->AudioStream);
        Assert::AreEqual(2u, reader->AllStreams->Size);
        Assert::IsFalse(reader->VideoStream->IsSelected);
        Assert::IsTrue(reader->AudioStream->IsSelected);

        while (true)
        {
            auto result = Await(reader->AudioStream->ReadAsync());
            if (result->EndOfStream)
            {
                Log() << L"EndOfStream reached";
                break;
            }
            if (result->Error)
            {
                Log() << L"Error received 0x" << hex << uppercase << setw(8) << setfill(L'0') << result->ErrorCode << L" " << result->ErrorMessage->Data();
                Assert::Fail();
            }
            Assert::IsNotNull(result->Sample);
        }
    }

    TEST_METHOD(CX_W_MediaReader_FromFile_LumiaEffect)
    {
        Log() << L"Creating MediaReader";
        auto reader = Await(MediaReader::CreateFromPathAsync(L"ms-appx:///car.mp4"));

        Log() << L"Reading sample";
        auto result = Await(reader->VideoStream->ReadAsync());
        auto inputSample = safe_cast<MediaSample2D^>(result->Sample);

        auto outputSample = ref new MediaSample2D(MediaSample2DFormat::Nv12, inputSample->Width, inputSample->Height);

        {
            auto inputBuffer = inputSample->LockBuffer(BufferAccessMode::Read);
            auto outputBuffer = outputSample->LockBuffer(BufferAccessMode::Write);

            Log() << L"Wrapping MediaBuffer2D in Bitmap";
            auto inputBitmap = ref new Bitmap(
                Size((float)inputSample->Width, (float)inputSample->Height),
                ColorMode::Yuv420Sp,
                ref new Array < unsigned int > { inputBuffer->Planes->GetAt(0)->Pitch, inputBuffer->Planes->GetAt(1)->Pitch },
                ref new Array < IBuffer^ > { inputBuffer->Planes->GetAt(0)->Buffer, inputBuffer->Planes->GetAt(1)->Buffer }
            );
            auto outputBitmap = ref new Bitmap(
                Size((float)outputSample->Width, (float)outputSample->Height),
                ColorMode::Yuv420Sp,
                ref new Array < unsigned int > { outputBuffer->Planes->GetAt(0)->Pitch, outputBuffer->Planes->GetAt(1)->Pitch },
                ref new Array < IBuffer^ > { outputBuffer->Planes->GetAt(0)->Buffer, outputBuffer->Planes->GetAt(1)->Buffer }
            );

            Log() << L"Applying effect";
            auto effect = ref new FilterEffect();
            effect->Filters = ref new Vector<IFilter^> { ref new SepiaFilter() };
            effect->Source = ref new BitmapImageSource(inputBitmap);
            auto renderer = ref new BitmapRenderer(effect, outputBitmap);
            Await(renderer->RenderAsync());
        }

        Log() << "Saving sample";
        auto folder = Await(KnownFolders::PicturesLibrary->CreateFolderAsync(L"MediaCaptureReaderTests", CreationCollisionOption::OpenIfExists));
        auto file = Await(folder->CreateFileAsync(L"CX_W_MediaReader_FromFile_LumiaEffect.jpg", CreationCollisionOption::ReplaceExisting));
        Await(ImageEncoder::SaveToFileAsync(outputSample, file, ImageCompression::Jpeg));
        Log() << L"Saved " << file->Path->Data();
    }

    //
    // Windows tests use NullMediaCapture as the real MediaCapture tries to pop up a consent UI which is nowhere to be seen
    // and cannot be automatically dismissed from within the tests
    //

    TEST_METHOD(CX_W_MediaReader_FromCapture_Basics)
    {
        Await(CoreApplication::MainView->CoreWindow->Dispatcher->RunAsync(
            CoreDispatcherPriority::Normal,
            ref new DispatchedHandler([]()
        {
            auto settings = ref new MediaCaptureInitializationSettings();
            settings->StreamingCaptureMode = StreamingCaptureMode::Video;

            auto capture = NullMediaCapture::Create();
            Await(capture->InitializeAsync(settings));

            auto graphicsDevice = MediaGraphicsDevice::CreateFromMediaCapture(capture);

            auto previewProps = (VideoEncodingProperties^)capture->VideoDeviceController->GetMediaStreamProperties(MediaStreamType::VideoPreview);

            auto image = ref new SurfaceImageSource(previewProps->Width, previewProps->Height);

            auto imagePresenter = ImagePresenter::CreateFromSurfaceImageSource(
                image,
                graphicsDevice,
                previewProps->Width,
                previewProps->Height
                );

            auto panel = ref new SwapChainPanel();
            auto swapChainPresenter = ImagePresenter::CreateFromSwapChainPanel(
                panel,
                graphicsDevice,
                previewProps->Width,
                previewProps->Height
                );

            auto mediaReader = Await(MediaReader::CreateFromMediaCaptureAsync(capture, AudioInitialization::Deselected, VideoInitialization::Bgra8));

            for (int n = 0; n < 3; n++)
            {
                Log() << "Displaying frame";
                auto result = Await(mediaReader->VideoStream->ReadAsync());
                auto sample = safe_cast<MediaSample2D^>(result->Sample);
                swapChainPresenter->Present(sample);
                imagePresenter->Present(sample);
            }
        })));
    }

    TEST_METHOD(CX_W_MediaReader_FromCapture_SaveBgra8ToJpeg)
    {
        auto capture = NullMediaCapture::Create();
        Await(capture->InitializeAsync());

        auto mediaReader = Await(MediaReader::CreateFromMediaCaptureAsync(capture, AudioInitialization::Deselected, VideoInitialization::Bgra8));

        Log() << "Capturing sample";
        auto result = Await(mediaReader->VideoStream->ReadAsync());
        auto sample = safe_cast<MediaSample2D^>(result->Sample);
        Assert::IsTrue(sample->Format == MediaSample2DFormat::Bgra8);

        Log() << "Saving sample";
        auto folder = Await(KnownFolders::PicturesLibrary->CreateFolderAsync(L"MediaCaptureReaderTests", CreationCollisionOption::OpenIfExists));
        auto file = Await(folder->CreateFileAsync(L"CX_W_MediaReader_FromCapture_SaveBgra8ToJpeg.jpg", CreationCollisionOption::ReplaceExisting));
        Await(ImageEncoder::SaveToFileAsync(sample, file, ImageCompression::Jpeg));
        Log() << L"Saved " << file->Path->Data();
    }

    TEST_METHOD(CX_W_MediaReader_FromCapture_SaveNv12ToJpeg)
    {
        auto capture = NullMediaCapture::Create();
        Await(capture->InitializeAsync());

        auto mediaReader = Await(MediaReader::CreateFromMediaCaptureAsync(capture, AudioInitialization::Deselected, VideoInitialization::Nv12));

        Log() << "Capturing sample";
        auto result = Await(mediaReader->VideoStream->ReadAsync());
        auto sample = safe_cast<MediaSample2D^>(result->Sample);
        Assert::IsTrue(sample->Format == MediaSample2DFormat::Nv12);

        Log() << "Saving sample";
        auto folder = Await(KnownFolders::PicturesLibrary->CreateFolderAsync(L"MediaCaptureReaderTests", CreationCollisionOption::OpenIfExists));
        auto file = Await(folder->CreateFileAsync(L"CX_W_MediaReader_FromCapture_SaveNv12ToJpeg.jpg", CreationCollisionOption::ReplaceExisting));
        Await(ImageEncoder::SaveToFileAsync(sample, file, ImageCompression::Jpeg));
        Log() << L"Saved " << file->Path->Data();
    }
};
