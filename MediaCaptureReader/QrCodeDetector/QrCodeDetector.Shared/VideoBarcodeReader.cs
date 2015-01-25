using MediaCaptureReader;
using System;
using System.Collections.Generic;
using System.Text;
using System.Threading;
using System.Threading.Tasks;
using Windows.System.Threading;
using ZXing;
using ZXing.Common;
using System.Runtime.InteropServices.WindowsRuntime;
using Windows.Foundation;

namespace QrCodeDetector
{
    class SampleDecodedEventArgs
    {
        /// <summary>Detected text. Null if no barcode detected.</summary>
        public String Text;

        /// <summary>Detected points in [0, 1] normalized coordinates. Empty if no barcode detected.</summary>
        public List<Point> Points = new List<Point>();
    }

    internal delegate void SampleDecodedHandler(VideoBarcodeReader sender, SampleDecodedEventArgs e);

    class VideoBarcodeReader : IDisposable
    {
        private volatile MediaSample2D m_sample;
        private ImageProcessor m_processor = new ImageProcessor(); // TODO: dispose
        private BarcodeReader m_barcodeReader = new BarcodeReader
        {
            Options = new DecodingOptions
            {
                PossibleFormats = new BarcodeFormat[] { BarcodeFormat.QR_CODE },
                TryHarder = true
            }
        };

        public event SampleDecodedHandler SampleDecoded;

        public void QueueSample(MediaSample2D sample)
        {
            lock (this)
            {
                if (m_sample == null)
                {
                    m_sample = sample;
                    Task.Run((Action)DecodeSample);
                }
                else
                {
                    sample.Dispose();
                    Logger.Events.VideoBarcodeReader_SampleDropped();
                }
            }
        }

        private void DecodeSample()
        {
            MediaSample2D sample;
            lock (this)
            {
                sample = m_sample;
            }
            if (sample == null)
            {
                return;
            }

            Logger.Events.VideoBarcodeReader_DecodeSampleStart();

            int scale = (int)Math.Round(Math.Max(sample.Width / 640.0, sample.Height / 480.0));

            var e = new SampleDecodedEventArgs();

            using (sample)
            using (var scaledSample = m_processor.Convert(sample, MediaSample2DFormat.Bgra8, sample.Width / scale, sample.Height / scale))
            using (var buffer = scaledSample.LockBuffer(BufferAccessMode.Read))
            {
                var plane = buffer.Planes[0];
                var result = m_barcodeReader.Decode(
                    plane.Buffer.ToArray(),
                    buffer.Width,
                    buffer.Height,
                    BitmapFormat.BGR32
                    );

                if (result != null)
                {
                    e.Text = result.Text;
                    for (int n = 0; n < result.ResultPoints.Length; n++)
                    {
                        e.Points.Add(new Point(result.ResultPoints[n].X / scaledSample.Width, result.ResultPoints[n].Y / scaledSample.Height));
                    }
                }
            }

            Logger.Events.VideoBarcodeReader_DecodeSampleStop(e.Text);

            lock (this)
            {
                m_sample = null;
            }

            if (SampleDecoded != null)
            {
                SampleDecoded(this, e);
            }
        }

        public void Dispose()
        {
            lock (this)
            {
                if (m_sample != null)
                {
                    m_sample.Dispose();
                    m_sample = null;
                }
            }
        }
    }
}
