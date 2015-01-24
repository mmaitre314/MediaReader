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

namespace QrCodeDetector
{
    internal delegate void SampleDecodedHandler(VideoBarcodeReader sender, Result e);

    class VideoBarcodeReader : IDisposable
    {
        private volatile MediaSample2D m_sample;
        private ImageProcessor m_processor = new ImageProcessor(); // TODO: dispose
        private BarcodeReader m_barcodeReader = new BarcodeReader
        {
            Options = new DecodingOptions
            {
                PossibleFormats = new BarcodeFormat[] { BarcodeFormat.QR_CODE }
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

            int scale = (int)Math.Round(Math.Max(sample.Width / 640.0, sample.Height / 480.0));

            Result result;

            using (sample)
            using (var scaledSample = m_processor.Convert(sample, MediaSample2DFormat.Bgra8, sample.Width / scale, sample.Height / scale))
            using (var buffer = scaledSample.LockBuffer(BufferAccessMode.Read))
            {
                result = m_barcodeReader.Decode(
                    buffer.Planes[0].Buffer.ToArray(),
                    buffer.Width,
                    buffer.Height,
                    BitmapFormat.BGR32
                    );
            }

            lock (this)
            {
                m_sample = null;
            }

            if (SampleDecoded != null)
            {
                SampleDecoded(this, result);
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
