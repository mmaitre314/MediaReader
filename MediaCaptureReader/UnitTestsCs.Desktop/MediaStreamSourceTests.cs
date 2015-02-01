using Microsoft.VisualStudio.TestTools.UnitTesting;
using System;
using System.Runtime.InteropServices.WindowsRuntime;
using System.Threading.Tasks;
using Windows.Media.Core;
using Windows.Media.MediaProperties;
using Windows.Media.Transcoding;
using Windows.Storage;

namespace UnitTestsCs.Desktop
{
    class AudioSampleGenerator
    {
        const int m_sampleRate = 44100;
        const double m_sineFrequency = 440;

        public AudioEncodingProperties EncodingProperties { get; private set; }

        // Start time of the next sample in seconds
        public TimeSpan Time { get; private set; }

        public AudioSampleGenerator()
        {
            EncodingProperties = AudioEncodingProperties.CreatePcm(m_sampleRate, 1, 16);
        }

        public MediaStreamSample GenerateSample()
        {
            // Generate 1s of data
            var buffer = new byte[2 * m_sampleRate];

            var time = Time.TotalSeconds;
            for (int i = 0; i < m_sampleRate; i++)
            {
                Int16 value = (Int16)(Int16.MaxValue * Math.Sin(2 * Math.PI * m_sineFrequency * time * time)); // Chirp sine wave

                buffer[2 * i] = (byte)(value & 0xFF);
                buffer[2 * i + 1] = (byte)((value >> 8) & 0xFF);

                time += (1 / (double)m_sampleRate);
            }

            var sample = MediaStreamSample.CreateFromBuffer(buffer.AsBuffer(), Time);
            sample.Discontinuous = (Time == TimeSpan.Zero);
            sample.Duration = TimeSpan.FromSeconds(1);
            Time += TimeSpan.FromSeconds(1);

            return sample;
        }
    }

    [TestClass]
    public class MediaStreamSourceTests
    {
        [TestMethod]
        public async Task CS_D_MediaStreamSource_EncodeAudio()
        {
            //
            // Create a PCM audio source
            //

            var generator = new AudioSampleGenerator();

            var source = new MediaStreamSource(
                new AudioStreamDescriptor(
                    generator.EncodingProperties
                    )
                );
            source.CanSeek = false;
            source.MusicProperties.Title = "CS_D_MediaStreamSource_EncodeAudio";

            source.SampleRequested += (MediaStreamSource sender, MediaStreamSourceSampleRequestedEventArgs args) =>
            {
                Console.WriteLine("SampleRequested Time: {0}", generator.Time);

                // Generate 5s of data
                if (generator.Time.TotalSeconds < 5)
                {
                    args.Request.Sample = generator.GenerateSample();
                }
            };

            //
            // Open destination M4A file
            //

            var musicFolder = await StorageFolder.GetFolderFromPathAsync(Environment.GetFolderPath(Environment.SpecialFolder.MyMusic));
            var folder = await musicFolder.CreateFolderAsync("MediaCaptureReaderTests", CreationCollisionOption.OpenIfExists);
            var file = await folder.CreateFileAsync("CS_D_MediaStreamSource_EncodeAudio.m4a", CreationCollisionOption.ReplaceExisting);
            Console.WriteLine("Output file: " + file.Path);

            using (var destination = await file.OpenAsync(FileAccessMode.ReadWrite))
            {
                //
                // Encode PCM to M4A
                //

                var transcoder = new MediaTranscoder();
                var result = await transcoder.PrepareMediaStreamSourceTranscodeAsync(
                    source,
                    destination,
                    MediaEncodingProfile.CreateM4a(AudioEncodingQuality.Medium)
                    );
                await result.TranscodeAsync();
            }
        }
    }
}
