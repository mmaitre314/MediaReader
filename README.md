[![Build status](https://ci.appveyor.com/api/projects/status/ix4eud7pf2w9p0gt?svg=true)](https://ci.appveyor.com/project/mmaitre314/mediacapturereader)
[![NuGet package](http://mmaitre314.github.io/images/nuget.png)](https://www.nuget.org/packages/MMaitre.MediaCaptureReader/)

Media Reader
============

`MediaReader` reads audio/video data from files (`StorageFile`, `IRandomAccessStream`, path), cameras and microphone (`MediaCapture`), and generic sources (`IMediaSource`). Universal Store apps targeting either Windows or Windows Phone are supported. A NuGet package is available [here](http://www.nuget.org/packages/MMaitre.MediaCaptureReader/).

For instance, `MediaReader` allows reading video frames from an MP4 file and saving them as a series of JPEG images:

![Photos](http://mmaitre314.github.io/images/CS_W_MediaReader_SaveAllFrameAsJpeg.JPG)

```c#
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
```

## Creating MediaReader

### From files

Three static methods are available to create `MediaReader` from `StorageFile`, `IRandomAccessStream`, and path respectively: `CreateFromFileAsync()`, `CreateFromStreamAsync()`, and `CreateFromPathAsync()`. `CreateFromPathAsync` in particular is useful to read files in the app package using the `ms-appx` scheme:

```c#
var reader = await MediaReader.CreateFromPathAsync("ms-appx:///car.mp4");
```

### From cameras and microphones

Data from cameras and microphones is read by creating `MediaCapture` and passing it to `MediaReader.CreateFromMediaCaptureAsync()`:

```c#
var capture = new MediaCapture();
await capture.InitializeAsync();

var reader = await MediaReader.CreateFromMediaCaptureAsync(capture);
```

A `SelectVideoDeviceAsync()` extension method is provided on `MediaCaptureInitializationSettings` to simplify selecting the back and front cameras:

```c#
using MediaCaptureReaderExtensions;

var settings = new MediaCaptureInitializationSettings
{
    // Video-only capture
    StreamingCaptureMode = StreamingCaptureMode.Video
};
if (!await settings.SelectVideoDeviceAsync(VideoDeviceSelection.BackOrFirst))
{
    // No camera
    return;
}

var capture = new MediaCapture();
await _capture.InitializeAsync(settings);
```

`SelectVideoDeviceAsync()` returns `false` if no camera is present.

### From generic media sources

`MediaReader` can also read data from `IMediaSource` using `CreateFromMediaSourceAsync()`. This can be used in conjunction with the `HttpMjpegCaptureSource` class in the same package, a media source streaming MJPEG videos from IP cameras:

```c#
var source = await HttpMjpegCaptureSource.CreateFromUriAsync("http://216.123.238.208/axis-cgi/mjpg/video.cgi?camera&resolution=640x480");
var reader = await MediaReader.CreateFromMediaSourceAsync(source.Source);
```

![Airport](http://mmaitre314.github.io/images/CS_W_MediaReader_IpCam.jpg)

## Reading audio/video data

### Selecting the stream format

The `AudioInitialization` and `VideoInitialization` enumerations control data processing in `MediaReader`. They can be used for instance to read the video data as Bgra8 while ignoring the audio data:

```c#
var reader = await MediaReader.CreateFromFileAsync(file, AudioInitialization.Deselected, VideoInitialization.Bgra8);
```

Audio streams can be initialized as `Pcm`, `PassThrough`, or `Deselected`, while video streams can be initialized as `Nv12`, `Bgra8`, `PassThrough`, or `Deselected`. The defaults are `Nv12` and `Pcm`. In `PassThrough` mode the data is provided in the format it is stored in the file, which is typically compressed (say H.264 and AAC).

### Reading data samples

The `AudioStream` and `VideoStream` properties on `MediaReader` provide `ReadAsync()` methods to read samples from the first audio and video streams in the file. The properties are null if the file has no audio or no video data. An `AllStreams` property also gives access to all the available streams.

```c#
using (var result = await _mediaReader.VideoStream.ReadAsync())
```

The `ReadAsync()` methods return `MediaReaderReadResult` objects which provide information about the stream state (`EndOfStream`, `Error`, etc. properties) and give out the data just read (`Sample` property). The `Sample` property is of type `IMediaSample`. As-is it gives access to the sample time and duration. It can also be cast to `MediaSample2D` when reading uncompressed video and `MediaSample1D` when reading compressed video or either compressed or uncompressed audio. The `LockBuffer()` methods on those two classes give access to the sample data.

```c#
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
```

## Processing 2D samples

`ImageProcessor` provides methods to convert the format/width/height of 2D samples and rotate them:

```c#
var processor = new ImageProcessor();

var sample1 = new MediaSample2D(MediaSample2DFormat.Nv12, 320, 240);
var sample2 = processor.Rotate(sample1, BitmapRotation.Clockwise90Degrees);
var sample3 = processor.Convert(sample2, MediaSample2DFormat.Bgra8, 480, 640);
```

## Saving 2D samples

`ImageEncoder` encodes 2D samples to JPEG:

```c#
var file = await KnownFolders.PicturesLibrary.CreateFileAsync("Image.jpg");
await ImageEncoder.SaveToFileAsync(sample, file, ImageCompression.Jpeg);
```

To simplify that code a bit, extension methods on `MediaSample2D` are also provided:

```c#
using MediaCaptureReaderExtensions;

await sample.SaveToFileAsync(file, ImageCompression.Jpeg);
```

## Displaying 2D samples

`ImagePresenter` displays 2D samples in XAML using either `SurfaceImageSource`

```c#
var image = new SurfaceImageSource((int)previewProps.Width, (int)previewProps.Height);
var imagePresenter = ImagePresenter.CreateFromSurfaceImageSource(image, graphicsDevice);
var imagePresenter.Present(sample);
```

or `SwapChainPanel`

```c#
var swapChainPresenter = ImagePresenter.CreateFromSwapChainPanel(
    SwapChainPreview,
    graphicsDevice,
    (int)previewProps.Width,
    (int)previewProps.Height
    );
swapChainPresenter.Present(sample);
```

## Examples

### Detecting QR codes using [ZXing.Net](http://www.nuget.org/packages/ZXing.Net/)

```c#
var barcodeReader = new BarcodeReader
{
    PossibleFormats = new BarcodeFormat[] { BarcodeFormat.QR_CODE }
};

using (var mediaReader = await MediaReader.CreateFromPathAsync("ms-appx:///QR_12345678.mp4", AudioInitialization.Deselected, VideoInitialization.Bgra8))
using (var mediaResult = await mediaReader.VideoStream.ReadAsync())
{
    var sample = (MediaSample2D)mediaResult.Sample;

    using (var buffer = sample.LockBuffer(BufferAccessMode.Read))
    {
        var barcodeResult = barcodeReader.Decode(
            buffer.Planes[0].Buffer.ToArray(),
            buffer.Width,
            buffer.Height,
            BitmapFormat.BGR32
            );

        ...
    }
}
```

### Applying effects using the [Lumia Imaging SDK](http://www.nuget.org/packages/LumiaImagingSDK/)

```c#
using (var mediaReader = await MediaReader.CreateFromPathAsync("ms-appx:///car.mp4", AudioInitialization.Deselected, VideoInitialization.Nv12))
using (var mediaResult = await mediaReader.VideoStream.ReadAsync())
{
    var inputSample = (MediaSample2D)mediaResult.Sample;

    using (var outputSample = new MediaSample2D(MediaSample2DFormat.Nv12, inputSample.Width, inputSample.Height))
    using (var inputBuffer = inputSample.LockBuffer(BufferAccessMode.Read))
    using (var outputBuffer = outputSample.LockBuffer(BufferAccessMode.Write))
    {
        // Wrap MediaBuffer2D in Bitmap
        var inputBitmap = new Bitmap(
            new Size(inputSample.Width, inputSample.Height), 
            ColorMode.Yuv420Sp,
            new uint[] { inputBuffer.Planes[0].Pitch, inputBuffer.Planes[1].Pitch },
            new IBuffer[] { inputBuffer.Planes[0].Buffer, inputBuffer.Planes[1].Buffer }
            );
        var outputBitmap = new Bitmap(
            new Size(inputSample.Width, inputSample.Height),
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
}
```

### Accessing pixel data

For efficiency, a `GetData()` extension method is added on `IBuffer`. `GetData()` returns an 'unsafe' `byte*` pointing to the `IBuffer` data. This requires methods calling `GetData()` to be marked using the `unsafe` keyword and to check the 'Allow unsafe code' checkbox in the project build properties.

The following code snippet reads RGB values and doubles them:

```c#
using MediaCaptureReaderExtensions;

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
```

### Streaming MJPEG videos from IP cameras

The `HttpMjpegCaptureSource` class streams MJPEG video from IP cameras. It can for instance be passed to XAML's `<MediaElement>` to preview the video stream:

```xml
    <MediaElement Name="VideoPreview" AutoPlay="True" RealTimePlayback="True" />
```

```c#
var source = await HttpMjpegCaptureSource.CreateFromUriAsync("http://216.123.238.208/axis-cgi/mjpg/video.cgi?camera&resolution=640x480");
VideoPreview.SetMediaStreamSource(source.Source);
```

## CaptureReader (deprecated)

`CaptureReader` reads audio/video samples from `MediaCapture`. The following code snippet reads a video sample in Bgra8 format from the default camera:

```c#
# Create MediaCapture
var capture = new MediaCapture();
await capture.InitializeAsync();

# Create a Bgra8 video format matching the camera resolution and framerate
var previewProps = (VideoEncodingProperties)capture.VideoDeviceController.GetMediaStreamProperties(MediaStreamType.VideoPreview);
var readerProps = VideoEncodingProperties.CreateUncompressed(MediaEncodingSubtypes.Bgra8, previewProps.Width, previewProps.Height);
readerProps.FrameRate.Numerator = previewProps.FrameRate.Numerator;
readerProps.FrameRate.Denominator = previewProps.FrameRate.Denominator;

# Create CaptureReader and get a video sample
var captureReader = await CaptureReader.CreateAsync(
    capture, 
    new MediaEncodingProfile
    {
        Video = readerProps
    });
MediaSample sample = await captureReader.GetVideoSampleAsync();
```
