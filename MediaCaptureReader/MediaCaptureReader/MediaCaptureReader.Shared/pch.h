#pragma once

#include <algorithm>
#include <sstream>
#include <queue>

#include <collection.h>
#include <ppltasks.h>

#include <strsafe.h>

#include <wrl.h>

#include <robuffer.h>

#include <d3d11_2.h>

#include <mfapi.h>
#include <mfidl.h>
#include <Mferror.h>

#include <windows.media.capture.h>
#include <windows.ui.xaml.media.dxinterop.h>

#include "DebuggerLogger.h"

namespace AWF = ::ABI::Windows::Foundation;
namespace AWFC = ::ABI::Windows::Foundation::Collections;
namespace AWM = ::ABI::Windows::Media;
namespace AWMC = ::ABI::Windows::Media::Capture;
namespace AWMD = ::ABI::Windows::Media::Devices;
namespace AWMMp = ::ABI::Windows::Media::MediaProperties;
namespace AWSS = ::ABI::Windows::Storage::Streams;
namespace MW = ::Microsoft::WRL;
namespace MWD = ::Microsoft::WRL::Details;
namespace MWW = ::Microsoft::WRL::Wrappers;
namespace WF = ::Windows::Foundation;
namespace WFM = ::Windows::Foundation::Metadata;
namespace WM = ::Windows::Media;
namespace WMC = ::Windows::Media::Capture;
namespace WMMp = ::Windows::Media::MediaProperties;
namespace WSS = ::Windows::Storage::Streams;
namespace WUXMI = ::Windows::UI::Xaml::Media::Imaging;
namespace WUXC = ::Windows::UI::Xaml::Controls;

//
// mfmediacapture.h is missing in Phone SDK 8.1
//

MIDL_INTERFACE("24E0485F-A33E-4aa1-B564-6019B1D14F65")
IAdvancedMediaCaptureSettings : public IUnknown
{
public:
    virtual HRESULT STDMETHODCALLTYPE GetDirectxDeviceManager(
        /* [out] */ IMFDXGIDeviceManager **value) = 0;

};

MIDL_INTERFACE("D0751585-D216-4344-B5BF-463B68F977BB")
IAdvancedMediaCapture : public IUnknown
{
public:
    virtual HRESULT STDMETHODCALLTYPE GetAdvancedMediaCaptureSettings(
        /* [out] */ __RPC__deref_out_opt IAdvancedMediaCaptureSettings **value) = 0;

};

//
// Error handling
//

// Exception-based error handling
#define CHK(statement)  {HRESULT _hr = (statement); if (FAILED(_hr)) { throw ref new Platform::COMException(_hr); };}
#define CHKNULL(p)  {if ((p) == nullptr) { throw ref new Platform::NullReferenceException(L#p); };}
#define CHKOOM(p)  {if ((p) == nullptr) { throw ref new Platform::OutOfMemoryException(L#p); };}

// Exception-free error handling
#define CHK_RETURN(statement) {hr = (statement); if (FAILED(hr)) { return hr; };}

//
// Error origin
//

namespace Details
{
    class ErrorOrigin
    {
    public:

        // A method to track error origin
        template <size_t N, size_t L>
        static HRESULT TracedOriginateError(_In_ char const (&function)[L], _In_ HRESULT hr, _In_ wchar_t const (&str)[N])
        {
            if (FAILED(hr))
            {
                s_logger.Log(function, LogLevel::Error, "failed hr=%08X: %S", hr, str);
                ::RoOriginateErrorW(hr, N - 1, str);
            }
            return hr;
        }

        // A method to track error origin
        template <size_t L>
        static HRESULT TracedOriginateError(_In_ char const (&function)[L], __in HRESULT hr)
        {
            if (FAILED(hr))
            {
                s_logger.Log(function, LogLevel::Error, "failed hr=%08X", hr);
                ::RoOriginateErrorW(hr, 0, nullptr);
            }
            return hr;
        }

        ErrorOrigin() = delete;
    };
}

// A method to track error origin
#define OriginateError(_hr, ...) ::Details::ErrorOrigin::TracedOriginateError(__FUNCTION__, _hr, __VA_ARGS__)

//
// Exception boundary (converts exceptions into HRESULTs)
//

namespace Details
{
    template<size_t L /*= sizeof(__FUNCTION__)*/>
    class TracedExceptionBoundary
    {
    public:
        TracedExceptionBoundary(_In_ const char *function /*= __FUNCTION__*/)
            : _function(function)
        {
        }

        TracedExceptionBoundary(const TracedExceptionBoundary&) = delete;
        TracedExceptionBoundary& operator=(const TracedExceptionBoundary&) = delete;

        HRESULT operator()(std::function<void()>&& lambda)
        {
            s_logger.Log(_function, L, LogLevel::Verbose, "boundary enter");

            HRESULT hr = S_OK;
            try
            {
                lambda();
            }
#ifdef _INC_COMDEF // include comdef.h to enable
            catch (const _com_error& e)
            {
                hr = e.Error();
            }
#endif
#ifdef __cplusplus_winrt // enable C++/CX to use (/ZW)
            catch (Platform::Exception^ e)
            {
                hr = e->HResult;
            }
#endif
            catch (const std::bad_alloc&)
            {
                hr = E_OUTOFMEMORY;
            }
            catch (const std::out_of_range&)
            {
                hr = E_BOUNDS;
            }
            catch (const std::exception& e)
            {
                s_logger.Log(_function, L, LogLevel::Error, "caught unknown STL exception: %s", e.what());
                hr = E_FAIL;
            }
            catch (...)
            {
                s_logger.Log(_function, L, LogLevel::Error, "caught unknown exception");
                hr = E_FAIL;
            }

            if (FAILED(hr))
            {
                s_logger.Log(_function, L, LogLevel::Error, "boundary exit - failed hr=%08X", hr);
            }
            else
            {
                s_logger.Log(_function, L, LogLevel::Verbose, "boundary exit");
            }

            return hr;
        }

    private:
        const char* _function;
    };
}

#define ExceptionBoundary ::Details::TracedExceptionBoundary<sizeof(__FUNCTION__)>(__FUNCTION__)

//
// Casting
//

template<typename T, typename U>
Microsoft::WRL::ComPtr<T> As(const Microsoft::WRL::ComPtr<U>& in)
{
    Microsoft::WRL::ComPtr<T> out;
    CHK(in.As(&out));
    return out;
}

template<typename T, typename U>
Microsoft::WRL::ComPtr<T> As(U* in)
{
    Microsoft::WRL::ComPtr<T> out;
    CHK(in->QueryInterface(IID_PPV_ARGS(&out)));
    return out;
}

template<typename T, typename U>
Microsoft::WRL::ComPtr<T> As(U^ in)
{
    Microsoft::WRL::ComPtr<T> out;
    CHK(reinterpret_cast<IInspectable*>(in)->QueryInterface(IID_PPV_ARGS(&out)));
    return out;
}

//
// IBuffer data access
//

inline unsigned char* GetData(Windows::Storage::Streams::IBuffer^ buffer)
{
    unsigned char* bytes = nullptr;
    Microsoft::WRL::ComPtr<::Windows::Storage::Streams::IBufferByteAccess> bufferAccess;
    CHK(((IUnknown*)buffer)->QueryInterface(IID_PPV_ARGS(&bufferAccess)));
    CHK(bufferAccess->Buffer(&bytes));
    return bytes;
}

//
// Exception-safe PROPVARIANT
//

class PropVariant : public PROPVARIANT
{
public:
    PropVariant()
    {
        PropVariantInit(this);
    }
    ~PropVariant()
    {
        (void)PropVariantClear(this);
    }

    PropVariant(const PropVariant&) = delete;
    PropVariant& operator&(const PropVariant&) = delete;
};

//
// Media Foundation startup/shutdown management
//

class AutoMF
{
public:

    AutoMF()
        : _initialized(false)
    {
        CHK(MFStartup(MF_VERSION));
        _initialized = true;
    }

    ~AutoMF()
    {
        if (_initialized)
        {
            (void)MFShutdown();
        }
    }

private:

    bool _initialized;
};

//
// Run code at scope exit
//
// Note: the lambda cannot throw as it is called in a destructor
//

namespace Details
{
    template<typename Lambda>
    class OnScopeExitImpl
    {
    public:

        OnScopeExitImpl(_In_ Lambda lambda)
            : _lambda(lambda)
        {
        }

        ~OnScopeExitImpl()
        {
            _lambda();
        }

    private:

        Lambda _lambda;
    };
}

template<typename Lambda>
::Details::OnScopeExitImpl<Lambda> OnScopeExit(_In_ Lambda lambda)
{
    return ::Details::OnScopeExitImpl<Lambda>(lambda);
}
