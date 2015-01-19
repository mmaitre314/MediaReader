using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using Windows.Storage;
using Windows.Storage.Streams;
using MediaCaptureReader;

namespace MediaCaptureReaderExtensions
{
    /// <summary>
    /// Extension methods on MediaSample encoding single images
    /// </summary>
    static public class MediaEncoder2DExtensions
    {
        /// <summary></summary>
        public static async Task SaveToFileAsync(this MediaSample2D sample, IStorageFile file, ImageCompression compression)
        {
            await MediaEncoder2D.SaveToFileAsync(sample, file, compression);
        }

        /// <summary></summary>
        public static async Task SaveToStreamAsync(this MediaSample2D sample, IRandomAccessStream stream, ImageCompression compression)
        {
            await MediaEncoder2D.SaveToStreamAsync(sample, stream, compression);
        }
    }
}
