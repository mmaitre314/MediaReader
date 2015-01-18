#pragma once

namespace MediaCaptureReader
{
    class WinRTBuffer WrlSealed : public MW::RuntimeClass <
        MW::RuntimeClassFlags<MW::RuntimeClassType::WinRtClassicComMix>,
        AWSS::IBuffer,
        MW::CloakedIid<WSS::IBufferByteAccess>,
        MW::FtmBase // if cross-process support is needed, replace with IMarshal from RoGetBufferMarshaler()
    >
    {
        InspectableClass(L"MediaCaptureReader.WinRTBuffer", TrustLevel::BaseTrust);

    public:

        WinRTBuffer(
            _In_reads_(_Inexpressible_) unsigned char *buffer,
            _In_ unsigned long capacity
            )
            : _buffer(buffer)
            , _capacity(capacity)
        {
        }

        void Close()
        {
            auto lock = _lock.LockExclusive();
            _buffer = nullptr;
            _capacity = 0;
        }

        //
        // IBuffer
        //

        IFACEMETHOD(get_Capacity)(_Out_ unsigned int *pValue) override
        {
            auto lock = _lock.LockShared();
            *pValue = _capacity;
            return S_OK;
        }

        IFACEMETHOD(get_Length)(_Out_ unsigned int *pValue) override
        {
            auto lock = _lock.LockShared();
            *pValue = _capacity;
            return S_OK;
        }

        IFACEMETHOD(put_Length)(_In_ unsigned int value) override
        {
            return OriginateError(E_ACCESSDENIED);
        }

        //
        // IBufferByteAccess
        //

        IFACEMETHOD(Buffer)(_Outptr_result_buffer_(_Inexpressible_("size given by different API")) unsigned char **ppValue) override
        {
            auto lock = _lock.LockShared();
            *ppValue = nullptr;

            if (_buffer == nullptr)
            {
                return OriginateError(RO_E_CLOSED);
            }

            *ppValue = _buffer;
            return S_OK;
        }

    private:

        virtual ~WinRTBuffer()
        {
        }

        unsigned char *_buffer;
        unsigned long _capacity;

        mutable MWW::SRWLock _lock;
    };
}