#include "pch.h"
#include "NullMediaCapture.h"
#include "NullMediaCaptureImpl.h"

using namespace MediaCaptureReader;
using namespace Windows::Media::Capture;
using namespace Microsoft::WRL;

MediaCapture^ NullMediaCapture::Create()
{
    auto capture = Make<NullMediaCaptureImpl>();
    CHKOOM(capture);

    return reinterpret_cast<MediaCapture^>(static_cast<AWMC::IMediaCapture*>(capture.Get()));
}
