#include "pch.h"

using namespace MediaCaptureReader;
using namespace Microsoft::VisualStudio::CppUnitTestFramework;
using namespace Microsoft::WRL;
using namespace Platform;
using namespace std;
using namespace Windows::ApplicationModel::Core;
using namespace Windows::Foundation;
using namespace Windows::Graphics::Imaging;
using namespace Windows::Media::Capture;
using namespace Windows::Media::MediaProperties;
using namespace Windows::Storage;
using namespace Windows::UI::Core;
using namespace Windows::UI::Xaml::Controls;
using namespace Windows::UI::Xaml::Media::Imaging;

TEST_CLASS(MediaReaderTests)
{
public:

    TEST_METHOD(CX_W_MediaSample_LockNv12)
    {
        auto reader = Await(MediaReader::CreateFromPathAsync(L"ms-appx:///car.mp4", AudioInitialization::Deselected, VideoInitialization::Nv12));

        Log() << "Reading sample";
        auto result = Await(reader->VideoStream->ReadAsync());
        auto sample = safe_cast<MediaSample2D^>(result->Sample);

        Log() << "Locking buffer";
        auto buffer = sample->LockBuffer(BufferAccessMode::Read);
        Assert::IsTrue(buffer->Format == MediaSample2DFormat::Nv12);
        Assert::AreEqual(320, buffer->Width);
        Assert::AreEqual(240, buffer->Height);
        Assert::AreEqual(2u, buffer->Planes->Size);
        Assert::IsNotNull(buffer->Planes->GetAt(0)->Buffer);
        Assert::IsTrue(buffer->Planes->GetAt(0)->Pitch > 0);
        Assert::IsTrue(buffer->Planes->GetAt(0)->Buffer->Capacity > 0);
        Assert::AreEqual(buffer->Planes->GetAt(0)->Buffer->Capacity, buffer->Planes->GetAt(0)->Buffer->Length);
        Assert::IsNotNull(buffer->Planes->GetAt(1)->Buffer);
        Assert::IsTrue(buffer->Planes->GetAt(1)->Pitch > 0);
        Assert::IsTrue(buffer->Planes->GetAt(1)->Buffer->Capacity > 0);
        Assert::AreEqual(buffer->Planes->GetAt(1)->Buffer->Capacity, buffer->Planes->GetAt(1)->Buffer->Length);
    }

    TEST_METHOD(CX_W_MediaSample_LockBgra8)
    {
        auto reader = Await(MediaReader::CreateFromPathAsync(L"ms-appx:///car.mp4", AudioInitialization::Deselected, VideoInitialization::Bgra8));

        Log() << "Reading sample";
        auto result = Await(reader->VideoStream->ReadAsync());
        auto sample = safe_cast<MediaSample2D^>(result->Sample);

        Log() << "Locking buffer";
        auto buffer = sample->LockBuffer(BufferAccessMode::Read);
        Assert::IsTrue(buffer->Format == MediaSample2DFormat::Bgra8);
        Assert::AreEqual(320, buffer->Width);
        Assert::AreEqual(240, buffer->Height);
        Assert::AreEqual(1u, buffer->Planes->Size);
        Assert::IsNotNull(buffer->Planes->GetAt(0)->Buffer);
        Assert::IsTrue(buffer->Planes->GetAt(0)->Pitch > 0);
        Assert::IsTrue(buffer->Planes->GetAt(0)->Buffer->Capacity > 0);
        Assert::AreEqual(buffer->Planes->GetAt(0)->Buffer->Capacity, buffer->Planes->GetAt(0)->Buffer->Length);
    }

};