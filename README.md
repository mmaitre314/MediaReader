[![Build status](https://ci.appveyor.com/api/projects/status/ix4eud7pf2w9p0gt?svg=true)](https://ci.appveyor.com/project/mmaitre314/mediacapturereader)

MediaCapture Reader
===================

Reads audio/video samples from cameras and microphone using MediaCapture. Store apps targeting either Windows or Windows Phone are supported. A NuGet package is available [here](http://www.nuget.org/packages/MMaitre.MediaCaptureReader/).

The following code snippet reads a video sample in Bgra8 format from the default camera:

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

The component also provides methods to display video samples in XAML using either SurfaceImageSource

```c#
var image = new SurfaceImageSource((int)previewProps.Width, (int)previewProps.Height);
var imagePresenter = MediaSamplePresenter.CreateFromSurfaceImageSource(image, graphicsDevice);
var imagePresenter.Present(sample);
```

or SwapChainPanel

```c#
var swapChainPresenter = MediaSamplePresenter.CreateFromSwapChainPanel(
    SwapChainPreview,
    graphicsDevice,
    (int)previewProps.Width,
    (int)previewProps.Height
    );
swapChainPresenter.Present(sample);
```
