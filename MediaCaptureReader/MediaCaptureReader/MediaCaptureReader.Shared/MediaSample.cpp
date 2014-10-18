#include "pch.h"
#include "MediaSample.h"

using namespace MediaCaptureReader;
using namespace Microsoft::WRL;
using namespace Platform;

MediaSample::MediaSample(_In_ const MW::ComPtr<IMFSample>& sample)
    : _sample(sample)
{
    assert(sample != nullptr);
}

MediaSample::~MediaSample()
{
}
