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
using Windows.Graphics.Imaging;

namespace UnitTestsCs
{
    [TestClass]
    public class CodeSnippetTests
    {
        [TestMethod]
        public async Task CS_W_MediaReader_SaveAllFrameAsJpeg()
        {
            var folder = await KnownFolders.PicturesLibrary.CreateFolderAsync("MediaCaptureReaderTests", CreationCollisionOption.OpenIfExists);
            folder = await folder.CreateFolderAsync("CS_W_MediaReader_SaveAllFrameAsJpeg", CreationCollisionOption.ReplaceExisting);

            using (var reader = await MediaReader.CreateFromPathAsync("ms-appx:///car.mp4"))
            {
                while (true)
                {
                    using (var result = await reader.VideoStream.ReadAsync())
                    {
                        if (result.EndOfStream || result.Error)
                        {
                            break;
                        }

                        var sample = (MediaSample2D)result.Sample;
                        if (sample == null)
                        {
                            continue;
                        }

                        var file = await folder.CreateFileAsync(((int)sample.Timestamp.TotalMilliseconds).ToString("D6") + ".jpg");
                        await sample.SaveToFileAsync(file, ImageCompression.Jpeg);
                    }
                }
            }
        }

        [TestMethod]
        public async Task CS_W_MediaReader_PixelDataAccess()
        {
            using (var reader = await MediaReader.CreateFromPathAsync("ms-appx:///car.mp4", AudioInitialization.Deselected, VideoInitialization.Bgra8))
            using (var result = await reader.VideoStream.ReadAsync())
            {
                var sample = (MediaSample2D)result.Sample;
                ProcessSample(sample);
            }
        }

        private unsafe void ProcessSample(MediaSample2D sample)
        {
            using (var buffer = sample.LockBuffer(BufferAccessMode.ReadWrite))
            {
                int width = buffer.Width;
                int height = buffer.Height;
                int pitch = (int)buffer.Planes[0].Pitch;
                byte* data = buffer.Planes[0].Buffer.GetData();

                for (int i = 0; i < height; i++)
                {
                    for (int j = 0; j < width; j++)
                    {
                        int pos = i * pitch + 4 * j;
                        data[pos + 0] = (byte)Math.Min(255, 2 * data[pos + 0]); // B
                        data[pos + 1] = (byte)Math.Min(255, 2 * data[pos + 1]); // G
                        data[pos + 2] = (byte)Math.Min(255, 2 * data[pos + 2]); // R
                    }
                }
            }
        }

        [TestMethod]
        public async Task CS_W_MediaReader_ReadAudio()
        {
            using (var reader = await MediaReader.CreateFromPathAsync("ms-appx:///Recording.m4a"))
            {
                while (true)
                {
                    using (var result = await reader.AudioStream.ReadAsync())
                    {
                        if (result.EndOfStream || result.Error)
                        {
                            break;
                        }

                        var sample = (MediaSample1D)result.Sample;
                        if (sample == null)
                        {
                            continue;
                        }

                        // Use audio data here
                    }
                }
            }
        }

        [TestMethod]
        public async Task CS_W_ImageProcessor_ImageEncoder()
        {
            var processor = new ImageProcessor();

            var sample1 = new MediaSample2D(MediaSample2DFormat.Nv12, 320, 240);
            var sample2 = processor.Rotate(sample1, BitmapRotation.Clockwise90Degrees);
            var sample3 = processor.Convert(sample2, MediaSample2DFormat.Bgra8, 480, 640);

            var file1 = await KnownFolders.PicturesLibrary.CreateFileAsync("CS_W_ImageProcessor_ImageEncoder1.jpg", CreationCollisionOption.ReplaceExisting);
            await ImageEncoder.SaveToFileAsync(sample1, file1, ImageCompression.Jpeg);

            var file2 = await KnownFolders.PicturesLibrary.CreateFileAsync("CS_W_ImageProcessor_ImageEncoder2.jpg", CreationCollisionOption.ReplaceExisting);
            await sample2.SaveToFileAsync(file2, ImageCompression.Jpeg);
        }

    }
}
