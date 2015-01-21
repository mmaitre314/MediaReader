[![Build status](https://ci.appveyor.com/api/projects/status/ix4eud7pf2w9p0gt?svg=true)](https://ci.appveyor.com/project/mmaitre314/mediacapturereader)
[![NuGet package](http://mmaitre314.github.io/images/nuget.png)](https://www.nuget.org/packages/MMaitre.MediaCaptureReader/)

-- Work in progress --

Media Reader
============

Reads audio/video data from files (`StorageFile`, `IRandomAccessStream`, path), cameras and microphone (`MediaCapture`) and generic sources (`IMediaSource`). Universal Store apps targeting either Windows or Windows Phone are supported. A NuGet package is available [here](http://www.nuget.org/packages/MMaitre.MediaCaptureReader/).

Save all the frames of a video to JPEG files:

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

## Create MediaReader

+++ init control (passthrough, etc.)

### From files

`CreateFromFileAsync` `StorageFile`
`CreateFromStreamAsync` `IRandomAccessStream`
`CreateFromPathAsync`

`CreateFromPathAsync` in particular is useful to read files in the app package using the `ms-appx` scheme:

```c#
var reader = await MediaReader.CreateFromPathAsync("ms-appx:///car.mp4");
```

### From cameras and microphones

`CreateFromMediaCaptureAsync`

```c#
var capture = new MediaCapture();
await capture.InitializeAsync();

var reader = await MediaReader.CreateFromMediaCaptureAsync(capture);
```

camera selection
`SelectVideoDeviceAsync` extension method allowing simple selection of back and front cameras.
The following code snippet selects the back camera if present or the first camera available on system without camera location information (ex: USB cameras on Desktop). `SelectVideoDeviceAsync` returns `false` if no cameras are present.

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

### From generic media sources

`CreateFromMediaSourceAsync`

```c#
var source = await HttpMjpegCaptureSource.CreateFromUriAsync("http://216.123.238.208/axis-cgi/mjpg/video.cgi?camera&resolution=640x480");
var reader = await MediaReader.CreateFromMediaSourceAsync(source.Source);
```

![Airport](http://mmaitre314.github.io/images/CS_W_MediaReader_IpCam.jpg)

## Read audio/video data

### Stream format

The `AudioInitialization` and `VideoInitialization` enumerations are used to control data processing in `MediaReader`. For instance, the following code snippet opens a file reading video data as Bgra8 and ignoring audio data:

```c#
var reader = await MediaReader.CreateFromFileAsync(file, AudioInitialization.Deselected, VideoInitialization.Bgra8);
```

Audio streams can be initialized as `Pcm`, `PassThrough`, or `Deselected`, while video streams can be initialized as `Nv12`, `Bgra8`, `PassThrough`, or `Deselected`. The defaults are `Nv12` and `Pcm`. In `PassThrough` mode the data is provided in the format it is stored in the file, which is typically compressed (say H.264 and AAC).

Compressed/uncompressed audio streams and compressed video streams: MediaSample1D
Uncompressed video streams: MediaStream2D

### Read samples

The `AudioStream` and `VideoStream` properties on `MediaReader` provide `ReadAsync()` methods to read samples from the first audio and video streams in the file. The properties are null if the file has no audio or no video data. An `AllStreams` property also gives access to all the available streams.

```c#
using (var result = await _mediaReader.VideoStream.ReadAsync())
```

The `ReadAsync()` methods return `MediaReaderVideoStream` objects which provide information about the stream state (`EndOfStream`, `Error`, etc. properties) and give out the sample just read (`Sample` property). The `Sample` property is of type `IMediaSample`, which gives access to the sample time and duration. To get access to the data it contains, `LockBuffer()` needs to be called after casting the sample to either `MediaSample1D` or `MediaSample2D`.

TODO: snippet Loop until EOS, handle errors
    
## Process 2D samples

`MediaProcessor2D` provides methods to convert the format/width/height of 2D samples and rotate them.

### Convert

TODO: code snippet

### Rotate

TODO: code snippet

## Save 2D samples

`MediaEncoder2D`

Also: C# extension method

## Display 2D samples

`MediaPresenter2D` provides methods to display 2D samples in XAML using either `SurfaceImageSource`

```c#
var image = new SurfaceImageSource((int)previewProps.Width, (int)previewProps.Height);
var imagePresenter = MediaPresenter2D.CreateFromSurfaceImageSource(image, graphicsDevice);
var imagePresenter.Present(sample);
```

or `SwapChainPanel`

```c#
var swapChainPresenter = MediaPresenter2D.CreateFromSwapChainPanel(
    SwapChainPreview,
    graphicsDevice,
    (int)previewProps.Width,
    (int)previewProps.Height
    );
swapChainPresenter.Present(sample);
```

## Scenarios

### Detect QR codes using ZXing.Net

### Apply effects using Lumia Imaging SDK

### Pixel data access

### Stream MJPEG videos from IP cameras

TODO: code snippet

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
