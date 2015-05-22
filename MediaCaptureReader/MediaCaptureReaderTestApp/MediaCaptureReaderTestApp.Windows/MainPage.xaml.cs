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
using Windows.Storage;
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
        MediaReader _mediaReader;
        ImagePresenter _imagePresenter;
        ImagePresenter _swapChainPresenter;

        public MainPage()
        {
            this.InitializeComponent();
        }

        protected override async void OnNavigatedTo(NavigationEventArgs e)
        {
            DisplayInformation.AutoRotationPreferences = DisplayOrientations.Landscape;

            VideoPreview.MediaFailed += VideoPreview_MediaFailed;
            //var file = await StorageFile.GetFileFromApplicationUriAsync(new Uri("ms-appx:///Assets/video.cvmpilj.mjpg"));
            //var stream = await file.OpenAsync(FileAccessMode.Read);
            //var source = await HttpMjpegCaptureSource.CreateFromStreamAsync(stream, "myboundary");
            var source = await HttpMjpegCaptureSource.CreateFromUriAsync("http://216.123.238.208/axis-cgi/mjpg/video.cgi?camera&resolution=640x480");
            VideoPreview.SetMediaStreamSource(source.Source);

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

            TextLog.Text += "Creating MediaReader\n";

            _mediaReader = await MediaReader.CreateFromMediaCaptureAsync(_capture, AudioInitialization.Deselected, VideoInitialization.Bgra8);

            TextLog.Text += "Starting video loop\n";

            var ignore = Task.Run(() => VideoLoop());
        }

        void VideoPreview_MediaFailed(object sender, ExceptionRoutedEventArgs e)
        {
            TextLog.Text += String.Format("VideoPreview MediaFailed: {0}\n", e.ErrorMessage);
        }

        async void VideoLoop()
        {
            while (true)
            {
                var result = await _mediaReader.VideoStream.ReadAsync().AsTask().ConfigureAwait(false);
                var sample = (MediaSample2D)result.Sample;

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
