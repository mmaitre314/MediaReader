#include "pch.h"
#include "NullMediaCaptureSettings.h"

using namespace ABI::Windows::Media::Capture;

NullMediaCaptureSettings::NullMediaCaptureSettings()
    : _streamingCaptureMode(StreamingCaptureMode_AudioAndVideo)
{
}

NullMediaCaptureSettings::~NullMediaCaptureSettings()
{
}
