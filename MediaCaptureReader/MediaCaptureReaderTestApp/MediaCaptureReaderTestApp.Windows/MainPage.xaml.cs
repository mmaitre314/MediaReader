using MediaCaptureReader;
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
        MediaSamplePresenter _imagePresenter;
        MediaSamplePresenter _swapChainPresenter;

        public MainPage()
        {
            this.InitializeComponent();
        }

        protected override async void OnNavigatedTo(NavigationEventArgs e)
        {
            DisplayInformation.AutoRotationPreferences = DisplayOrientations.Landscape;

            _capture = new MediaCapture();
            await _capture.InitializeAsync(new MediaCaptureInitializationSettings
            {
                VideoDeviceId = await GetBackOrDefaulCameraIdAsync(),
                StreamingCaptureMode = StreamingCaptureMode.Video
            });

            var graphicsDevice = MediaGraphicsDevice.CreateFromMediaCapture(_capture);

            var previewProps = (VideoEncodingProperties)_capture.VideoDeviceController.GetMediaStreamProperties(MediaStreamType.VideoPreview);
            TextLog.Text += String.Format("Preview: {0} {1}x{2} {3}fps\n", previewProps.Subtype, previewProps.Width, previewProps.Height, previewProps.FrameRate.Numerator / (float)previewProps.FrameRate.Denominator);

            TextLog.Text += "Creating MediaSamplePresenter from SurfaceImageSource\n";

            var image = new SurfaceImageSource((int)previewProps.Width, (int)previewProps.Height);
            ImagePreview.Source = image;
            _imagePresenter = MediaSamplePresenter.CreateFromSurfaceImageSource(image, graphicsDevice, (int)previewProps.Width, (int)previewProps.Height);

            TextLog.Text += "Creating MediaSamplePresenter from SwapChainPanel\n";

            _swapChainPresenter = MediaSamplePresenter.CreateFromSwapChainPanel(
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
                MediaSample sample = await _captureReader.GetVideoSampleAsync().AsTask().ConfigureAwait(false);

                _swapChainPresenter.Present(sample);

                var ignore = Dispatcher.RunAsync(CoreDispatcherPriority.Normal, () =>
                    {
                        _imagePresenter.Present(sample);
                        sample.Dispose();
                    });
            }
        }

        public static async Task<string> GetBackOrDefaulCameraIdAsync()
        {
            var devices = await DeviceInformation.FindAllAsync(DeviceClass.VideoCapture);

            string deviceId = "";

            foreach (var device in devices)
            {
                if ((device.EnclosureLocation != null) &&
                    (device.EnclosureLocation.Panel == Windows.Devices.Enumeration.Panel.Back))
                {
                    deviceId = device.Id;
                    break;
                }
            }

            return deviceId;
        }
    }
}
