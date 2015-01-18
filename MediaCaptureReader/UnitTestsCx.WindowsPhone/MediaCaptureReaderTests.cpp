#include "pch.h"

using namespace MediaCaptureReader;
using namespace Microsoft::VisualStudio::CppUnitTestFramework;
using namespace Windows::ApplicationModel::Core;
using namespace Windows::Media::Capture;
using namespace Windows::Media::MediaProperties;
using namespace Windows::UI::Core;
using namespace Windows::UI::Xaml::Controls;
using namespace Windows::UI::Xaml::Media::Imaging;

TEST_CLASS(MediaCaptureReaderTests)
{
public:

    TEST_METHOD(CX_WP_MediaCaptureReader_Basic)
    {
        Await(CoreApplication::MainView->CoreWindow->Dispatcher->RunAsync(
            CoreDispatcherPriority::Normal,
            ref new DispatchedHandler([]()
        {
            auto settings = ref new MediaCaptureInitializationSettings();
            settings->StreamingCaptureMode = StreamingCaptureMode::Video;

            auto capture = ref new MediaCapture();
            Await(capture->InitializeAsync(settings));

            auto graphicsDevice = MediaGraphicsDevice::CreateFromMediaCapture(capture);

            auto previewProps = (VideoEncodingProperties^)capture->VideoDeviceController->GetMediaStreamProperties(MediaStreamType::VideoPreview);

            auto image = ref new SurfaceImageSource(previewProps->Width, previewProps->Height);

            auto imagePresenter = MediaSample2DPresenter::CreateFromSurfaceImageSource(
                image,
                graphicsDevice,
                previewProps->Width,
                previewProps->Height
                );

            auto panel = ref new SwapChainPanel();
            auto swapChainPresenter = MediaSample2DPresenter::CreateFromSwapChainPanel(
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

            auto sample = safe_cast<MediaSample2D^>(Await(captureReader->GetVideoSampleAsync()));
            swapChainPresenter->Present(sample);
            imagePresenter->Present(sample);
        })));
    }

};
