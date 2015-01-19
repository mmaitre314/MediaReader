#include "pch.h"

using namespace MediaCaptureReader;
using namespace Microsoft::VisualStudio::CppUnitTestFramework;
using namespace Windows::ApplicationModel::Core;
using namespace Windows::Media::Capture;
using namespace Windows::Media::MediaProperties;
using namespace Windows::Storage;
using namespace Windows::UI::Core;
using namespace Windows::UI::Xaml::Controls;
using namespace Windows::UI::Xaml::Media::Imaging;

TEST_CLASS(CaptureReaderTests)
{
public:

    //
    // Windows tests use NullMediaCapture as the real MediaCapture tries to pop up a consent UI which is nowhere to be seen
    // and cannot be automatically dismissed from within the tests
    //

    TEST_METHOD(CX_W_CaptureReader_Basic)
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

            auto imagePresenter = MediaPresenter2D::CreateFromSurfaceImageSource(
                image,
                graphicsDevice,
                previewProps->Width,
                previewProps->Height
                );

            auto panel = ref new SwapChainPanel();
            auto swapChainPresenter = MediaPresenter2D::CreateFromSwapChainPanel(
                panel,
                graphicsDevice,
                previewProps->Width,
                previewProps->Height
                );

            auto readerProps = VideoEncodingProperties::CreateUncompressed(MediaEncodingSubtypes::Bgra8, previewProps->Width, previewProps->Height);
            readerProps->FrameRate->Numerator = previewProps->FrameRate->Numerator;
            readerProps->FrameRate->Denominator = previewProps->FrameRate->Denominator;

            auto profile = ref new MediaEncodingProfile();
            profile->Video = readerProps;

            auto captureReader = Await(CaptureReader::CreateAsync(capture, profile));

            for (int n = 0; n < 3; n++)
            {
                Log() << "Displaying frame";
                auto sample = safe_cast<MediaSample2D^>(Await(captureReader->GetVideoSampleAsync()));
                swapChainPresenter->Present(sample);
                imagePresenter->Present(sample);
            }
        })));
    }

    TEST_METHOD(CX_W_CaptureReader_SaveBgra8ToJpeg)
    {
        auto capture = NullMediaCapture::Create();
        Await(capture->InitializeAsync());

        auto previewProps = (VideoEncodingProperties^)capture->VideoDeviceController->GetMediaStreamProperties(MediaStreamType::VideoPreview);

        auto readerProps = VideoEncodingProperties::CreateUncompressed(MediaEncodingSubtypes::Bgra8, previewProps->Width, previewProps->Height);
        readerProps->FrameRate->Numerator = previewProps->FrameRate->Numerator;
        readerProps->FrameRate->Denominator = previewProps->FrameRate->Denominator;

        auto profile = ref new MediaEncodingProfile();
        profile->Video = readerProps;

        auto captureReader = Await(CaptureReader::CreateAsync(capture, profile));

        Log() << "Saving sample";
        auto sample = safe_cast<MediaSample2D^>(Await(captureReader->GetVideoSampleAsync()));
        auto folder = Await(KnownFolders::PicturesLibrary->CreateFolderAsync(L"MediaCaptureReaderTests", CreationCollisionOption::OpenIfExists));
        auto file = Await(folder->CreateFileAsync(L"CX_W_CaptureReader_SaveBgra8ToJpeg.jpg", CreationCollisionOption::ReplaceExisting));
        Await(MediaEncoder2D::SaveToFileAsync(sample, file, ImageCompression::Jpeg));
        Log() << L"Saved " << file->Path->Data();
    }

    TEST_METHOD(CX_W_CaptureReader_SaveNv12ToJpeg)
    {
        auto capture = NullMediaCapture::Create();
        Await(capture->InitializeAsync());

        auto previewProps = (VideoEncodingProperties^)capture->VideoDeviceController->GetMediaStreamProperties(MediaStreamType::VideoPreview);

        auto readerProps = VideoEncodingProperties::CreateUncompressed(MediaEncodingSubtypes::Nv12, previewProps->Width, previewProps->Height);
        readerProps->FrameRate->Numerator = previewProps->FrameRate->Numerator;
        readerProps->FrameRate->Denominator = previewProps->FrameRate->Denominator;

        auto profile = ref new MediaEncodingProfile();
        profile->Video = readerProps;

        auto captureReader = Await(CaptureReader::CreateAsync(capture, profile));

        Log() << "Saving sample";
        auto sample = safe_cast<MediaSample2D^>(Await(captureReader->GetVideoSampleAsync()));
        auto folder = Await(KnownFolders::PicturesLibrary->CreateFolderAsync(L"MediaCaptureReaderTests", CreationCollisionOption::OpenIfExists));
        auto file = Await(folder->CreateFileAsync(L"CX_W_CaptureReader_SaveNv12ToJpeg.jpg", CreationCollisionOption::ReplaceExisting));
        Await(MediaEncoder2D::SaveToFileAsync(sample, file, ImageCompression::Jpeg));
        Log() << L"Saved " << file->Path->Data();
    }

};
