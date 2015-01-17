#pragma once

namespace MediaCaptureReader
{

    class SourceReaderCallback :
        public Microsoft::WRL::RuntimeClass <
        Microsoft::WRL::RuntimeClassFlags<Microsoft::WRL::ClassicCom>,
        IMFSourceReaderCallback,
        Microsoft::WRL::FtmBase
        >
    {
    public:

        HRESULT RuntimeClassInitialize(_In_ Platform::WeakReference parent)
        {
            _parent = parent;
            return S_OK;
        }

        IFACEMETHOD(OnReadSample)(
            _In_ HRESULT hrStatus,
            _In_ DWORD dwStreamIndex,
            _In_ DWORD dwStreamFlags,
            _In_ LONGLONG llTimestamp,
            _In_opt_ IMFSample *pSample
            ) override
        {
            auto parent = _parent.Resolve<MediaReaderSharedState>();
            if (parent != nullptr)
            {
                parent->OnReadSample(hrStatus, dwStreamIndex, (MF_SOURCE_READER_FLAG)dwStreamFlags, llTimestamp, pSample);
            }
            return S_OK;
        }

        IFACEMETHOD(OnFlush)(_In_  DWORD dwStreamIndex) override
        {
            return S_OK;
        }

        IFACEMETHOD(OnEvent)(_In_  DWORD dwStreamIndex, _In_ IMFMediaEvent *pEvent) override
        {
            return S_OK;
        }

    private:

        Platform::WeakReference _parent;
    };

}