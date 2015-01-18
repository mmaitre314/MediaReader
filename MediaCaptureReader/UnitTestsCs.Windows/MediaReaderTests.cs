using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using Microsoft.VisualStudio.TestPlatform.UnitTestFramework;
using MediaCaptureReader;
using System.Threading.Tasks;
using ZXing;
using Lumia.Imaging;
using Lumia.Imaging.Artistic;
using Windows.Foundation;
using Windows.Storage;
using System.Runtime.InteropServices.WindowsRuntime;
using MediaCaptureReaderExtensions;
using Windows.Storage.Streams;

namespace UnitTestsCs.Windows
{
    [TestClass]
    public class MediaReaderTests
    {
        [TestMethod]
        public async Task CS_W_MediaReader_TestZXing()
        {
            var barcodeReader = new BarcodeReader
            {
                PossibleFormats = new BarcodeFormat[] { BarcodeFormat.QR_CODE }
            };

            using (var mediaReader = await MediaReader.CreateFromPathAsync("ms-appx:///QR_12345678.mp4", AudioInitialization.Deselected, VideoInitialization.Bgra8))
            using (var mediaResult = await mediaReader.VideoStreams[0].ReadAsync())
            {
                var sample = (MediaSample2D)mediaResult.Sample;
                Assert.AreEqual(MediaSample2DFormat.Bgra8, sample.Format);
                Assert.AreEqual(320, sample.Width);
                Assert.AreEqual(180, sample.Height);

                using (var buffer = sample.LockBuffer(BufferAccessMode.Read))
                {
                    var barcodeResult = barcodeReader.Decode(
                        buffer.Planes[0].Buffer.ToArray(),
                        buffer.Width,
                        buffer.Height,
                        BitmapFormat.BGR32
                        );

                    Assert.IsNotNull(barcodeResult);
                    Assert.AreEqual("12345678", barcodeResult.Text);
                }
            }
        }

        [TestMethod]
        public async Task CS_W_MediaReader_TestLumiaEffect()
        {
            using (var mediaReader = await MediaReader.CreateFromPathAsync("ms-appx:///car.mp4", AudioInitialization.Deselected, VideoInitialization.Nv12))
            using (var mediaResult = await mediaReader.VideoStreams[0].ReadAsync())
            {
                var streamProperties = mediaReader.VideoStreams[0].GetCurrentStreamProperties();
                int width = (int)streamProperties.Width;
                int height = (int)streamProperties.Height;
                Assert.AreEqual(320, width);
                Assert.AreEqual(240, height);

                var inputSample = (MediaSample2D)mediaResult.Sample;
                Assert.AreEqual(MediaSample2DFormat.Nv12, inputSample.Format);
                Assert.AreEqual(320, inputSample.Width);
                Assert.AreEqual(240, inputSample.Height);

                using (var outputSample = new MediaSample2D(MediaSample2DFormat.Nv12, width, height))
                {
                    Assert.AreEqual(MediaSample2DFormat.Nv12, outputSample.Format);
                    Assert.AreEqual(320, outputSample.Width);
                    Assert.AreEqual(240, outputSample.Height);

                    using (var inputBuffer = inputSample.LockBuffer(BufferAccessMode.Read))
                    using (var outputBuffer = outputSample.LockBuffer(BufferAccessMode.Write))
                    {
                        // Wrap MediaBuffer2D in Bitmap
                        var inputBitmap = new Bitmap(
                            new Size(width, height), 
                            ColorMode.Yuv420Sp,
                            new uint[] { inputBuffer.Planes[0].Pitch, inputBuffer.Planes[1].Pitch },
                            new IBuffer[] { inputBuffer.Planes[0].Buffer, inputBuffer.Planes[1].Buffer }
                            );
                        var outputBitmap = new Bitmap(
                            new Size(width, height),
                            ColorMode.Yuv420Sp,
                            new uint[] { outputBuffer.Planes[0].Pitch, outputBuffer.Planes[1].Pitch },
                            new IBuffer[] { outputBuffer.Planes[0].Buffer, outputBuffer.Planes[1].Buffer }
                            );

                        // Apply effect
                        var effect = new FilterEffect();
                        effect.Filters = new IFilter[] { new WatercolorFilter() };
                        effect.Source = new BitmapImageSource(inputBitmap);
                        var renderer = new BitmapRenderer(effect, outputBitmap);
                        await renderer.RenderAsync();
                    }

                    // Save the file
                    var folder = await KnownFolders.PicturesLibrary.CreateFolderAsync("MediaCaptureReaderTests", CreationCollisionOption.OpenIfExists);
                    var file = await folder.CreateFileAsync("CS_W_MediaReader_TestLumiaEffect.jpg", CreationCollisionOption.ReplaceExisting);
                    await outputSample.SaveToFileAsync(file, ImageCompression.Jpeg);
                    Logger.LogMessage("Saved {0}", file.Path);
                }
            }
        }
    }
}
