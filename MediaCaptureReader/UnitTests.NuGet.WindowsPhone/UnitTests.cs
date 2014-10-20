using System;
using Microsoft.VisualStudio.TestPlatform.UnitTestFramework;
using System.Threading.Tasks;
using Windows.Media.Capture;
using MediaCaptureReader;
using Windows.UI.Xaml.Media.Imaging;
using Windows.Media.MediaProperties;
using Windows.UI.Xaml.Controls;
using Windows.ApplicationModel.Core;
using Windows.UI.Core;
using System.Threading;

namespace UnitTests.NuGet.WindowsPhone
{
    [TestClass]
    public class UnitTests
    {
        [TestMethod]
        public void CS_WP_N_Basic()
        {
            ExecuteOnUIThread(async () =>
                {
                    var capture = new MediaCapture();
                    await capture.InitializeAsync(new MediaCaptureInitializationSettings
                    {
                        StreamingCaptureMode = StreamingCaptureMode.Video
                    });

                    var graphicsDevice = MediaGraphicsDevice.CreateFromMediaCapture(capture);

                    var previewProps = (VideoEncodingProperties)capture.VideoDeviceController.GetMediaStreamProperties(MediaStreamType.VideoPreview);

                    var image = new SurfaceImageSource((int)previewProps.Width, (int)previewProps.Height);

                    var imagePresenter = MediaSamplePresenter.CreateFromSurfaceImageSource(image, graphicsDevice);

                    var panel = new SwapChainPanel();
                    var swapChainPresenter = MediaSamplePresenter.CreateFromSwapChainPanel(
                        panel,
                        graphicsDevice,
                        (int)previewProps.Width,
                        (int)previewProps.Height
                        );

                    var readerProps = VideoEncodingProperties.CreateUncompressed(MediaEncodingSubtypes.Bgra8, previewProps.Width, previewProps.Height);
                    readerProps.FrameRate.Numerator = previewProps.FrameRate.Numerator;
                    readerProps.FrameRate.Denominator = previewProps.FrameRate.Denominator;

                    var captureReader = await CaptureReader.CreateAsync(
                        capture, new MediaEncodingProfile
                        {
                            Video = readerProps
                        });

                    using (MediaSample sample = await captureReader.GetVideoSampleAsync())
                    {
                        swapChainPresenter.Present(sample);
                        imagePresenter.Present(sample);
                    }
                });
        }

        private void ExecuteOnUIThread(Func<Task> op)
        {
            _exception = null;
            _event = new AutoResetEvent(false);

            var ignore = CoreApplication.MainView.CoreWindow.Dispatcher.RunAsync(
                CoreDispatcherPriority.Normal, async () =>
                {
                    try
                    {
                        await op();
                    }
                    catch (Exception ex)
                    {
                        _exception = ex;
                    }

                    _event.Set();
                });

            _event.WaitOne();

            if (_exception != null)
            {
                throw new Exception("", _exception);
            }
        }

        AutoResetEvent _event;
        Exception _exception;
    }
}
