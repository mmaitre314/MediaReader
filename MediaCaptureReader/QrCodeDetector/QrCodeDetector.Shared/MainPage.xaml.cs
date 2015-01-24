using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Runtime.InteropServices.WindowsRuntime;
using System.Threading.Tasks;
using Windows.Foundation;
using Windows.Foundation.Collections;
using Windows.Graphics.Display;
using Windows.Media;
using Windows.Media.Capture;
using Windows.System.Display;
using Windows.UI.Core;
using Windows.UI.Xaml;
using Windows.UI.Xaml.Controls;
using Windows.UI.Xaml.Controls.Primitives;
using Windows.UI.Xaml.Data;
using Windows.UI.Xaml.Input;
using Windows.UI.Xaml.Media;
using Windows.UI.Xaml.Navigation;
using MediaCaptureReader;
using MediaCaptureReaderExtensions;
using Windows.Media.MediaProperties;
using Windows.System.Threading;
using System.Collections.Concurrent;
using System.Threading;
using ZXing;

namespace QrCodeDetector
{
    public sealed partial class MainPage : Page
    {
        DisplayRequest m_displayRequest = new DisplayRequest();
        MediaCapture m_capture;
        MediaReader m_reader;
        ImagePresenter m_presenter;
        volatile MediaSample2D m_sample;
        AutoResetEvent m_sampleReadyEvent = new AutoResetEvent(false);
#if !WINDOWS_PHONE_APP
        SystemMediaTransportControls m_mediaControls;
#endif

        public MainPage()
        {
            this.InitializeComponent();

#if WINDOWS_PHONE_APP
            this.NavigationCacheMode = NavigationCacheMode.Required;
#endif
        }

        protected override void OnNavigatedTo(NavigationEventArgs e)
        {
            // Disable app UI rotation
            DisplayInformation.AutoRotationPreferences = DisplayOrientations.Landscape;

            // Prevent screen timeout
            m_displayRequest.RequestActive();

            // Handle app going to and coming out of background
#if WINDOWS_PHONE_APP
            Application.Current.Resuming += App_Resuming;
            Application.Current.Suspending += App_Suspending;
            var ignore = InitializeCaptureAsync();
#else
            m_mediaControls = SystemMediaTransportControls.GetForCurrentView();
            m_mediaControls.PropertyChanged += m_mediaControls_PropertyChanged;

            if (!IsInBackground())
            {
                var ignore = InitializeCaptureAsync();
            }
#endif
        }

        protected override void OnNavigatedFrom(NavigationEventArgs e)
        {
#if WINDOWS_PHONE_APP
            Application.Current.Resuming -= App_Resuming;
            Application.Current.Suspending -= App_Suspending;
#else
            m_mediaControls.PropertyChanged -= m_mediaControls_PropertyChanged;
#endif

            DisplayInformation.AutoRotationPreferences = DisplayOrientations.None;

            m_displayRequest.RequestRelease();

        }

#if WINDOWS_PHONE_APP
        private void App_Resuming(object sender, object e)
        {
            // Dispatch call to the UI thread since the event may get fired on some other thread
            var ignore = Dispatcher.RunAsync(CoreDispatcherPriority.Normal, async () =>
            {
                await InitializeCaptureAsync();
            });
        }

        private void App_Suspending(object sender, Windows.ApplicationModel.SuspendingEventArgs e)
        {
            DisposeCapture();
        }
#else
        void m_mediaControls_PropertyChanged(SystemMediaTransportControls sender, SystemMediaTransportControlsPropertyChangedEventArgs args)
        {
            if (args.Property != SystemMediaTransportControlsProperty.SoundLevel)
            {
                return;
            }

            if (!IsInBackground())
            {
                // Dispatch call to the UI thread since the event may get fired on some other thread
                var ignore = Dispatcher.RunAsync(CoreDispatcherPriority.Normal, async () =>
                {
                    await InitializeCaptureAsync();
                });
            }
            else
            {
                DisposeCapture();
            }
        }

        private bool IsInBackground()
        {
            return m_mediaControls.SoundLevel == SoundLevel.Muted;
        }
#endif

        private async Task InitializeCaptureAsync()
        {
            var settings = new MediaCaptureInitializationSettings
            {
                StreamingCaptureMode = StreamingCaptureMode.Video
            };
            await settings.SelectVideoDeviceAsync(VideoDeviceSelection.BackOrFirst);

            var capture = new MediaCapture();
            await capture.InitializeAsync(settings);

            // Select the capture resolution closest to screen resolution
            var formats = capture.VideoDeviceController.GetAvailableMediaStreamProperties(MediaStreamType.VideoPreview);
            var format = (VideoEncodingProperties)formats.OrderBy((item) =>
                {
                    var props = (VideoEncodingProperties)item;
                    return Math.Abs(props.Width - this.ActualWidth) + Math.Abs(props.Height - this.ActualHeight);
                }).First();
            await capture.VideoDeviceController.SetMediaStreamPropertiesAsync(MediaStreamType.VideoPreview, format);

            // Make the SwapChainPanel full screen
            double scale = Math.Min(this.ActualWidth / format.Width, this.ActualHeight / format.Height);
            Preview.Width = format.Width;
            Preview.Height = format.Height;
            Preview.RenderTransformOrigin = new Point(.5, .5);
            Preview.RenderTransform = new ScaleTransform { ScaleX = scale, ScaleY = scale };

            var reader = await MediaReader.CreateFromMediaCaptureAsync(capture, AudioInitialization.Deselected, VideoInitialization.Bgra8);
            var presenter = ImagePresenter.CreateFromSwapChainPanel(Preview, reader.GraphicsDevice, (int)format.Width, (int)format.Height);
            var sampleQueue = new BlockingCollection<MediaSample2D>();

            m_capture = capture;
            m_reader = reader;
            m_presenter = presenter;

            // Run preview/detection out of UI thread
            var ignore = ThreadPool.RunAsync(PreviewLoop);
            ignore = ThreadPool.RunAsync(AnalysisLoop);
        }

        private void DisposeCapture()
        {
            lock (this)
            {
                if (m_sample != null)
                {
                    m_sample.Dispose();
                    m_sample = null;
                }
                if (m_reader != null)
                {
                    m_reader.Dispose();
                    m_reader = null;
                }
                if (m_capture != null)
                {
                    m_capture.Dispose();
                    m_capture = null;
                }
                if (m_presenter != null)
                {
                    m_presenter.Dispose();
                    m_presenter = null;
                }
            }
        }

        private async void PreviewLoop(IAsyncAction operation)
        {
            var reader = m_reader;
            var presenter = m_presenter;
            
            try
            {
                while (true)
                {
                    using (var result = await reader.VideoStream.ReadAsync())
                    {
                        if (result.Error)
                        {
                            break;
                        }

                        var sample = (MediaSample2D)result.Sample;
                        if (sample == null)
                        {
                            continue;
                        }

                        presenter.Present(sample);
                        
                        lock (this)
                        {
                            if (m_sample == null)
                            {
                                m_sample = sample;
                                result.DetachSample();
                                m_sampleReadyEvent.Set();
                            }
                        }
                    }
                }
            }
            catch (Exception)
            {
            }
        }

        private void AnalysisLoop(IAsyncAction operation)
        {
            var processor = new ImageProcessor(); // TODO: dispose
            var barcodeReader = new BarcodeReader
            {
                PossibleFormats = new BarcodeFormat[] { BarcodeFormat.QR_CODE }
            };

            try
            {
                while (true)
                {
                    m_sampleReadyEvent.WaitOne();

                    MediaSample2D sample;
                    lock (this)
                    {
                        sample = m_sample;
                    }
                    if (sample == null)
                    {
                        continue;
                    }

                    int scale = (int)Math.Round(Math.Max(sample.Width / 640.0, sample.Height / 480.0));

                    using (sample)
                    using (var scaledSample = processor.Convert(sample, MediaSample2DFormat.Bgra8, sample.Width / scale, sample.Height / scale))
                    using (var buffer = scaledSample.LockBuffer(BufferAccessMode.Read))
                    {
                        var barcode = barcodeReader.Decode(
                            buffer.Planes[0].Buffer.ToArray(),
                            buffer.Width,
                            buffer.Height,
                            BitmapFormat.BGR32
                            );

                        var ignore = Dispatcher.RunAsync(CoreDispatcherPriority.Normal, () =>
                            {
                                TextLog.Text = barcode == null ? "No barcode" : barcode.Text;
                            });
                    }

                    lock (this)
                    {
                        m_sample = null;
                    }
                }
            }
            catch (Exception)
            {
            }
        }
    }
}
