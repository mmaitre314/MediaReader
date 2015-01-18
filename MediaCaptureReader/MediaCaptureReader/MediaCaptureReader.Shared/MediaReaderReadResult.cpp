#include "pch.h"
#include <roerrorapi.h>
#include "MediaReaderReadResult.h"
#include "MediaSample.h"

using namespace Platform;
using namespace MediaCaptureReader;
using namespace Microsoft::WRL;

MediaReaderReadResult::~MediaReaderReadResult()
{
    delete dynamic_cast<MediaSample1D^>(_sample);
    delete dynamic_cast<MediaSample2D^>(_sample);
    _sample = nullptr;
}

String^ MediaReaderReadResult::ErrorMessage::get()
{
    if (SUCCEEDED(_hr))
    {
        return L"";
    }

    String^ errorMessage;

#if WINAPI_PARTITION_PC_APP // SysFreeString() not available on Phone
    //
    // Get the error message from the RestrictedErrorInfo, if present
    //

    ComPtr<IRestrictedErrorInfo> error;
    (void)GetRestrictedErrorInfo(&error);

    if (error != nullptr)
    {
        BSTR description = nullptr;
        HRESULT errorHr = S_OK;
        BSTR restrictedDescription = nullptr;
        BSTR capabilitySid = nullptr;

        if (SUCCEEDED(error->GetErrorDetails(&description, &errorHr, &restrictedDescription, &capabilitySid)) && (errorHr == _hr))
        {
            if (wcscmp(description, restrictedDescription) == 0)
            {
                errorMessage = ref new String(description);
            }
            else
            {
                errorMessage = ref new String(description) + L" (" + ref new String(restrictedDescription) + L")";
            }

            SysFreeString(description);
            SysFreeString(capabilitySid);
            SysFreeString(restrictedDescription);
        }

        (void)SetRestrictedErrorInfo(error.Get());
    }
#endif

    //
    // If the error message missing, try some fallbacks
    //

    if (errorMessage->IsEmpty())
    {
        WCHAR buffer[MAX_ERROR_MESSAGE_CHARS + 1] = {};
        if (FormatMessage(
            FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
            nullptr,
            _hr,
            LANG_USER_DEFAULT,
            buffer,
            ARRAYSIZE(buffer),
            nullptr
            ))
        {
            errorMessage = ref new String(buffer);
        }
        else
        {
            errorMessage = L"Failed";
        }
    }

    return errorMessage;
}
