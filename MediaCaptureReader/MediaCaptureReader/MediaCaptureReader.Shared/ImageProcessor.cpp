#include "pch.h"
#include "MediaSample.h"
#include "ImageProcessor.h"
#include "MediaGraphicsDevice.h"

using namespace MediaCaptureReader;
using namespace Microsoft::WRL;
using namespace Platform;
using namespace Windows::Graphics::Imaging;

// A partially-functional MF Source (just enough to get SourceReader started)
class PlaceHolderVideoSource :
    public RuntimeClass <
    RuntimeClassFlags<ClassicCom>,
    IMFMediaEventGenerator,
    IMFMediaSource
    >
{
public:

    PlaceHolderVideoSource::PlaceHolderVideoSource(_In_ const ComPtr<IMFMediaType>& mediaType)
    {
        IMFMediaType* arrayTypes[] = { mediaType.Get() };
        ComPtr<IMFStreamDescriptor> streamDescr;
        CHK(MFCreateStreamDescriptor(0, ARRAYSIZE(arrayTypes), arrayTypes, &streamDescr));

        CHK(MFCreatePresentationDescriptor(1, streamDescr.GetAddressOf(), &_presDescr));
    }

    //
    // IMFMediaEventGenerator
    //

    IFACEMETHOD(GetEvent)(_In_ DWORD dwFlags, _COM_Outptr_ IMFMediaEvent **ppEvent) override
    {
        RoFailFastWithErrorContext(E_NOTIMPL);
        return E_NOTIMPL;
    }

    IFACEMETHOD(BeginGetEvent)(_In_ IMFAsyncCallback *pCallback, _In_ IUnknown *punkState) override
    {
        return S_OK;
    }

    IFACEMETHOD(EndGetEvent)(_In_ IMFAsyncResult *pResult, _COM_Outptr_  IMFMediaEvent **ppEvent) override
    {
        RoFailFastWithErrorContext(E_NOTIMPL);
        return E_NOTIMPL;
    }

    IFACEMETHOD(QueueEvent)(
        _In_ MediaEventType met,
        _In_ REFGUID guidExtendedType,
        _In_ HRESULT hrStatus,
        _In_opt_ const PROPVARIANT *pvValue
        ) override
    {
        RoFailFastWithErrorContext(E_NOTIMPL);
        return E_NOTIMPL;
    }


    //
    // IMFMediaSource
    //

    IFACEMETHOD(GetCharacteristics)(_Out_ DWORD *pdwCharacteristics)
    {
        *pdwCharacteristics = 0;
        return S_OK;
    }

    IFACEMETHOD(CreatePresentationDescriptor)(_COM_Outptr_  IMFPresentationDescriptor **ppPresentationDescriptor)
    {
        return _presDescr.CopyTo(ppPresentationDescriptor);
    }

    IFACEMETHOD(Start)(
        _In_opt_ IMFPresentationDescriptor *pPresentationDescriptor,
        _In_opt_ const GUID *pguidTimeFormat,
        _In_opt_ const PROPVARIANT *pvarStartPosition
        )
    {
        RoFailFastWithErrorContext(E_NOTIMPL);
        return E_NOTIMPL;
    }

    IFACEMETHOD(Stop)()
    {
        RoFailFastWithErrorContext(E_NOTIMPL);
        return E_NOTIMPL;
    }

    IFACEMETHOD(Pause)()
    {
        RoFailFastWithErrorContext(E_NOTIMPL);
        return E_NOTIMPL;
    }

    IFACEMETHOD(Shutdown)()
    {
        return S_OK;
    }

private:

    ComPtr<IMFPresentationDescriptor> _presDescr;
};

// An RAII version of MFT_OUTPUT_DATA_BUFFER
class MftOutputDataBuffer : public MFT_OUTPUT_DATA_BUFFER
{
public:

    MftOutputDataBuffer(_In_opt_ const ComPtr<IMFSample>& sample)
    {
        this->dwStreamID = 0;
        this->pSample = sample.Get();
        this->dwStatus = 0;
        this->pEvents = nullptr;

        if (this->pSample != nullptr)
        {
            this->pSample->AddRef();
        }
    }

    ~MftOutputDataBuffer()
    {
        if (this->pSample != nullptr)
        {
            this->pSample->Release();
        }
        if (this->pEvents != nullptr)
        {
            this->pEvents->Release();
        }
    }
};

ImageProcessor::ImageProcessor()
    : _inputFormat(MediaSample2DFormat::Unknown)
    , _outputFormat(MediaSample2DFormat::Unknown)
    , _rotation(BitmapRotation::None)
    , _outputWidth(0)
    , _outputHeight(0)
    , _inputWidth(0)
    , _inputHeight(0)
    , _usingGraphicsDevice(false)
{
    ZeroMemory(&_outputStreamInfo, sizeof(_outputStreamInfo));

    _CreateVideoProcessor();
}

void ImageProcessor::_CreateVideoProcessor()
{
    //
    // Create two different formats to force the SourceReader to create a video processor
    //

    ComPtr<IMFMediaType> outputFormat;
    CHK(MFCreateMediaType(&outputFormat));
    CHK(outputFormat->SetGUID(MF_MT_MAJOR_TYPE, MFMediaType_Video));
    CHK(outputFormat->SetGUID(MF_MT_SUBTYPE, MFVideoFormat_NV12));
    CHK(outputFormat->SetUINT32(MF_MT_INTERLACE_MODE, MFVideoInterlace_Progressive));
    CHK(MFSetAttributeSize(outputFormat.Get(), MF_MT_FRAME_SIZE, 640, 480));
    CHK(MFSetAttributeRatio(outputFormat.Get(), MF_MT_FRAME_RATE, 1, 1));
    CHK(MFSetAttributeRatio(outputFormat.Get(), MF_MT_PIXEL_ASPECT_RATIO, 1, 1));

    ComPtr<IMFMediaType> inputFormat;
    CHK(MFCreateMediaType(&inputFormat));
    CHK(outputFormat->CopyAllItems(inputFormat.Get()));
    CHK(MFSetAttributeSize(outputFormat.Get(), MF_MT_FRAME_SIZE, 800, 600));

    //
    // Create the SourceReader
    //

    auto source = Make<PlaceHolderVideoSource>(inputFormat);
    CHKOOM(source);

    ComPtr<IMFAttributes> sourceReaderAttr;
    CHK(MFCreateAttributes(&sourceReaderAttr, 2));
    CHK(sourceReaderAttr->SetUINT32(MF_SOURCE_READER_ENABLE_ADVANCED_VIDEO_PROCESSING, true));
    CHK(sourceReaderAttr->SetUINT32(MF_READWRITE_DISABLE_CONVERTERS, false));

    ComPtr<IMFReadWriteClassFactory> readerFactory;
    ComPtr<IMFSourceReaderEx> sourceReader;
    CHK(CoCreateInstance(CLSID_MFReadWriteClassFactory, nullptr, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&readerFactory)));
    CHK(readerFactory->CreateInstanceFromObject(
        CLSID_MFSourceReader, 
        static_cast<IMFMediaSource*>(source.Get()),
        sourceReaderAttr.Get(), 
        IID_PPV_ARGS(&sourceReader)
        ));

    CHK(sourceReader->SetCurrentMediaType(MF_SOURCE_READER_FIRST_VIDEO_STREAM, nullptr, outputFormat.Get()));

    //
    // Extract its video processor
    //

    unsigned int n = 0;
    while (true)
    {
        GUID category;
        ComPtr<IMFTransform> transform;
        CHK(sourceReader->GetTransformForStream(MF_SOURCE_READER_FIRST_VIDEO_STREAM, n, &category, &transform));

        // Only care about MFTs which are video processors and D3D11 aware
        ComPtr<IMFAttributes> transformAttr;
        if ((category == MFT_CATEGORY_VIDEO_PROCESSOR)
            && SUCCEEDED(transform->GetAttributes(&transformAttr))
            && (MFGetAttributeUINT32(transformAttr.Get(), MF_SA_D3D11_AWARE, 0) != 0)
            )
        {
            _processor = transform;
            break;
        }

        n++;
    }
}

MediaSample2D^ ImageProcessor::Convert(
    _In_ MediaSample2D^ sample,
    _In_ MediaSample2DFormat format,
    _In_ int width,
    _In_ int height
    )
{
    auto lock = _lock.LockExclusive();

    // Validate inputs
    CHKNULL(sample);
    if (format == MediaSample2DFormat::Unknown)
    {
        throw ref new InvalidArgumentException(L"format");
    }
    if (width <= 0)
    {
        throw ref new InvalidArgumentException(L"width");
    }
    if (height <= 0)
    {
        throw ref new InvalidArgumentException(L"height");
    }

    // Reinitialize the video processor if input/output conversion parameters changed
    if ((sample->Format != _inputFormat) ||
        (sample->Width != _inputWidth) ||
        (sample->Height != _inputHeight) ||
        (format != _outputFormat) ||
        (width != _outputWidth) ||
        (height != _outputHeight) ||
        (_rotation != BitmapRotation::None) ||
        (!Object::ReferenceEquals(sample->GraphicsDevice, _graphicsDevice))
        )
    {
        _inputFormat = sample->Format;
        _inputWidth = sample->Width;
        _inputHeight = sample->Height;
        _outputFormat = format;
        _outputWidth = width;
        _outputHeight = height;
        _rotation = BitmapRotation::None;
        _graphicsDevice = sample->GraphicsDevice;

        _InitializeVideoProcessor();
    }

    return _Process(sample);
}

MediaSample2D^ ImageProcessor::Rotate(
    _In_ MediaSample2D^ sample,
    _In_ BitmapRotation rotation
    )
{
    auto lock = _lock.LockExclusive();
    CHKNULL(sample);

    // Output parameters
    auto format = sample->Format;
    int width;
    int height;
    switch (rotation)
    {
    case BitmapRotation::Clockwise90Degrees:
    case BitmapRotation::Clockwise270Degrees:
        width = sample->Height;
        height = sample->Width;
        break;
    case BitmapRotation::Clockwise180Degrees:
        width = sample->Width;
        height = sample->Height;
        break;
    default: throw ref new InvalidArgumentException(L"rotation");
    }

    // Reinitialize the video processor if input/output conversion parameters changed
    if ((sample->Format != _inputFormat) ||
        (sample->Width != _inputWidth) ||
        (sample->Height != _inputHeight) ||
        (format != _outputFormat) ||
        (width != _outputWidth) ||
        (height != _outputHeight) ||
        (_rotation != rotation) ||
        (!Object::ReferenceEquals(sample->GraphicsDevice, _graphicsDevice))
        )
    {
        _inputFormat = sample->Format;
        _inputWidth = sample->Width;
        _inputHeight = sample->Height;
        _outputFormat = format;
        _outputWidth = width;
        _outputHeight = height;
        _rotation = rotation;
        _graphicsDevice = sample->GraphicsDevice;

        _InitializeVideoProcessor();
    }

    return _Process(sample);
}

void ImageProcessor::_InitializeVideoProcessor()
{
    // Create input media type
    ComPtr<IMFMediaType> inputType;
    CHK(MFCreateMediaType(&inputType));
    CHK(inputType->SetGUID(MF_MT_MAJOR_TYPE, MFMediaType_Video));
    CHK(inputType->SetGUID(MF_MT_SUBTYPE, MediaSample2D::GetSubtypeFromFormat(_inputFormat)));
    CHK(inputType->SetUINT32(MF_MT_INTERLACE_MODE, MFVideoInterlace_Progressive));
    CHK(MFSetAttributeSize(inputType.Get(), MF_MT_FRAME_SIZE, _inputWidth, _inputHeight));
    CHK(MFSetAttributeRatio(inputType.Get(), MF_MT_FRAME_RATE, 1, 1));
    CHK(MFSetAttributeRatio(inputType.Get(), MF_MT_PIXEL_ASPECT_RATIO, 1, 1));

    MFVideoRotationFormat rotation = MFVideoRotationFormat_0;
    switch (_rotation)
    {
    case BitmapRotation::Clockwise90Degrees: rotation = MFVideoRotationFormat_90; break;
    case BitmapRotation::Clockwise180Degrees: rotation = MFVideoRotationFormat_180; break;
    case BitmapRotation::Clockwise270Degrees: rotation = MFVideoRotationFormat_270; break;
    }
    CHK(inputType->SetUINT32(MF_MT_VIDEO_ROTATION, rotation));

    // Create output media type
    ComPtr<IMFMediaType> outputType;
    CHK(MFCreateMediaType(&outputType));
    CHK(outputType->SetGUID(MF_MT_MAJOR_TYPE, MFMediaType_Video));
    CHK(outputType->SetGUID(MF_MT_SUBTYPE, MediaSample2D::GetSubtypeFromFormat(_outputFormat)));
    CHK(outputType->SetUINT32(MF_MT_INTERLACE_MODE, MFVideoInterlace_Progressive));
    CHK(MFSetAttributeSize(outputType.Get(), MF_MT_FRAME_SIZE, _outputWidth, _outputHeight));
    CHK(MFSetAttributeRatio(outputType.Get(), MF_MT_FRAME_RATE, 1, 1));
    CHK(MFSetAttributeRatio(outputType.Get(), MF_MT_PIXEL_ASPECT_RATIO, 1, 1));

    // Set the input/output formats
    bool useGraphicsDevice = (_graphicsDevice != nullptr);
    if (useGraphicsDevice)
    {
        _usingGraphicsDevice = true;
        CHK(_processor->ProcessMessage(MFT_MESSAGE_SET_D3D_MANAGER, reinterpret_cast<ULONG_PTR>(_graphicsDevice->GetDeviceManager().Get())));

        HRESULT hrOutput = S_OK;
        HRESULT hrInput = _processor->SetInputType(0, inputType.Get(), 0);
        if (SUCCEEDED(hrInput))
        {
            hrOutput = _processor->SetOutputType(0, outputType.Get(), 0);
        }

        // Fall back on software if media types were rejected
        if (FAILED(hrInput) || FAILED(hrOutput))
        {
            useGraphicsDevice = false;
        }
    }
    if (!useGraphicsDevice)
    {
        _usingGraphicsDevice = false;
        CHK(_processor->ProcessMessage(MFT_MESSAGE_SET_D3D_MANAGER, 0));

        CHK(_processor->SetInputType(0, inputType.Get(), 0));
        CHK(_processor->SetOutputType(0, outputType.Get(), 0));
    }

    CHK(_processor->GetOutputStreamInfo(0, &_outputStreamInfo));
}

MediaSample2D^ ImageProcessor::_Process(_In_ MediaSample2D^ sample) const
{
    ComPtr<IMFMediaBuffer> inputBuffer1D;
    CHK(sample->GetSample()->GetBufferByIndex(0, &inputBuffer1D));

    // Input MF sample
    ComPtr<IMFSample> inputSample;
    CHK(MFCreateSample(&inputSample));
    CHK(inputSample->AddBuffer(inputBuffer1D.Get()));
    CHK(inputSample->SetSampleTime(0));
    CHK(inputSample->SetSampleDuration(10000000));

    // Output MF sample
    // In SW mode, we allocate the output buffer
    // In HW mode, the video proc allocates
    ComPtr<IMFSample> outputSample;
    if (!(_outputStreamInfo.dwFlags & MFT_OUTPUT_STREAM_PROVIDES_SAMPLES))
    {
        ComPtr<IMFMediaBuffer> buffer1D;
        CHK(MFCreate2DMediaBuffer(_outputWidth, _outputHeight, MediaSample2D::GetSubtypeFromFormat(_outputFormat).Data1, false, &buffer1D));

        CHK(MFCreateSample(&outputSample));
        CHK(outputSample->AddBuffer(buffer1D.Get()));
        CHK(outputSample->SetSampleTime(0));
        CHK(outputSample->SetSampleDuration(10000000));
    }

    // Process data
    DWORD status;
    MftOutputDataBuffer output(outputSample);
    CHK(_processor->ProcessMessage(MFT_MESSAGE_COMMAND_FLUSH, 0));
    CHK(_processor->ProcessInput(0, inputSample.Get(), 0));
    CHK(_processor->ProcessOutput(0, 1, &output, &status));

    // Copy MF sample properties
    int64 time = 0;
    int64 duration = 0;
    (void)sample->GetSample()->GetSampleTime(&time);
    (void)sample->GetSample()->GetSampleDuration(&duration);
    CHK(output.pSample->SetSampleTime(time));
    CHK(output.pSample->SetSampleDuration(duration));
    CHK(sample->GetSample()->CopyAllItems(output.pSample));

    // Wrap the MF sample in a MediaSample2D
    auto outputMediaSample = ref new MediaSample2D(
        output.pSample,
        _outputFormat,
        _outputWidth,
        _outputHeight,
        _usingGraphicsDevice ? sample->GraphicsDevice : nullptr
        );

    return outputMediaSample;
}
