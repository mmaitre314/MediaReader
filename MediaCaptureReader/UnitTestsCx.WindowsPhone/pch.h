#pragma once

#include "CppUnitTest.h"

#include <collection.h>
#include <sstream>
#include <iomanip>
#include <ppltasks.h>

#include "Await.h"

namespace Microsoft {
    namespace VisualStudio {
        namespace CppUnitTestFramework {

            template<> static std::wstring ToString<__int64>(const __int64& t) { RETURN_WIDE_STRING(t); }

            template<> static std::wstring ToString<MediaCaptureReader::MediaSample2DFormat>(const MediaCaptureReader::MediaSample2DFormat& t)
            {
                switch (t)
                {
                case MediaCaptureReader::MediaSample2DFormat::Bgra8: return L"Bgra8";
                case MediaCaptureReader::MediaSample2DFormat::Nv12: return L"Nv12";
                case MediaCaptureReader::MediaSample2DFormat::Unknown: return L"Unknown";
                case MediaCaptureReader::MediaSample2DFormat::Yuy2: return L"Yuy2";
                case MediaCaptureReader::MediaSample2DFormat::Yv12: return L"Yv12";
                default: return L"Unknown MediaSample2DFormat";
                }
            }

        }
    }
}

class Log : public std::wostringstream
{
public:
    Log()
    {
    }

    ~Log()
    {
        *this << std::endl;
        ::Microsoft::VisualStudio::CppUnitTestFramework::Logger::WriteMessage(this->str().c_str());
    }
};
