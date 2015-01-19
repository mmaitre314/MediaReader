[![Build status](https://ci.appveyor.com/api/projects/status/ix4eud7pf2w9p0gt?svg=true)](https://ci.appveyor.com/project/mmaitre314/mediacapturereader)
[![NuGet package](http://mmaitre314.github.io/images/nuget.png)](https://www.nuget.org/packages/MMaitre.MediaCaptureReader/)

Media Reader
============

Reads audio/video data from files (`StorageFile`, `IRandomAccessStream`, path), cameras and microphone (`MediaCapture`) and generic sources (`IMediaSource`). Universal Store apps targeting either Windows or Windows Phone are supported. A NuGet package is available [here](http://www.nuget.org/packages/MMaitre.MediaCaptureReader/).

## Create MediaReader

### From file

TODO: code snippet

### From stream

TODO: code snippet

### From path

TODO: code snippet

### From camera/microphone

TODO: code snippet

camera selection

TODO: code snippet

### From generic sources

TODO: code snippet

## Read audio/video data

Default format: Nv12, Pcm
Compressed/uncompressed audio streams and compressed video streams: MediaSample1D
Uncompressed video streams: MediaStream2D

### Read uncompressed audio samples

### Read uncompressed video samples
    
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
