using MediaCaptureReader;
using MediaCaptureReaderExtensions;
using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Runtime.InteropServices.WindowsRuntime;
using System.Threading.Tasks;
using Windows.Devices.Enumeration;
using Windows.Foundation;
using Windows.Foundation.Collections;
using Windows.Graphics.Display;
using Windows.Media.Capture;
using Windows.Media.MediaProperties;
using Windows.UI.Core;
using Windows.UI.Xaml;
using Windows.UI.Xaml.Controls;
using Windows.UI.Xaml.Controls.Primitives;
using Windows.UI.Xaml.Data;
using Windows.UI.Xaml.Input;
using Windows.UI.Xaml.Media;
using Windows.UI.Xaml.Media.Imaging;
using Windows.UI.Xaml.Navigation;

namespace MediaCaptureReaderTestApp
{
    public sealed partial class MainPage : Page
    {
        MediaCapture _capture;
        CaptureReader _captureReader;
        ImagePresenter _imagePresenter;
        ImagePresenter _swapChainPresenter;

        public MainPage()
        {
            this.InitializeComponent();

            this.NavigationCacheMode = NavigationCacheMode.Required;
        }

        protected override async void OnNavigatedTo(NavigationEventArgs e)
        {
            DisplayInformation.AutoRotationPreferences = DisplayOrientations.Landscape;

            var settings = new MediaCaptureInitializationSettings
            {
                StreamingCaptureMode = StreamingCaptureMode.Video
            };
            await settings.SelectVideoDeviceAsync(VideoDeviceSelection.BackOrFirst);

            _capture = new MediaCapture();
            await _capture.InitializeAsync(settings);

            var graphicsDevice = MediaGraphicsDevice.CreateFromMediaCapture(_capture);

            var previewProps = (VideoEncodingProperties)_capture.VideoDeviceController.GetMediaStreamProperties(MediaStreamType.VideoPreview);
            TextLog.Text += String.Format("Preview: {0} {1}x{2} {3}fps\n", previewProps.Subtype, previewProps.Width, previewProps.Height, previewProps.FrameRate.Numerator / (float)previewProps.FrameRate.Denominator);

            TextLog.Text += "Creating MediaSamplePresenter from SurfaceImageSource\n";

            var image = new SurfaceImageSource((int)previewProps.Width, (int)previewProps.Height);
            ImagePreview.Source = image;
            _imagePresenter = ImagePresenter.CreateFromSurfaceImageSource(image, graphicsDevice, (int)previewProps.Width, (int)previewProps.Height);

            TextLog.Text += "Creating MediaSamplePresenter from SwapChainPanel\n";

            _swapChainPresenter = ImagePresenter.CreateFromSwapChainPanel(
                SwapChainPreview,
                graphicsDevice,
                (int)previewProps.Width,
                (int)previewProps.Height
                );

            TextLog.Text += "Creating CaptureReader\n";

            var readerProps = VideoEncodingProperties.CreateUncompressed(MediaEncodingSubtypes.Bgra8, previewProps.Width, previewProps.Height);
            readerProps.FrameRate.Numerator = previewProps.FrameRate.Numerator;
            readerProps.FrameRate.Denominator = previewProps.FrameRate.Denominator;

            _captureReader = await CaptureReader.CreateAsync(
                _capture, new MediaEncodingProfile
                {
                    Video = readerProps
                });

            TextLog.Text += "Starting video loop\n";

            var ignore = Task.Run(() => VideoLoop());
        }

        async void VideoLoop()
        {
            while (true)
            {
                var sample = (MediaSample2D)await _captureReader.GetVideoSampleAsync().AsTask().ConfigureAwait(false);

                _swapChainPresenter.Present(sample);

                var ignore = Dispatcher.RunAsync(CoreDispatcherPriority.Normal, () =>
                {
                    _imagePresenter.Present(sample);
                    sample.Dispose();
                });
            }
        }
    }
}
