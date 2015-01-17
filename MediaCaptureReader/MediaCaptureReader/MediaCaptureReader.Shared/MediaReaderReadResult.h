#pragma once

namespace MediaCaptureReader
{
    ref class MediaSample;

    [WFM::GCPressure(amount = WFM::GCPressureAmount::High)]
    public ref class MediaReaderReadResult sealed
    {
    public:

        // IClosable
        // Without it an expression like result.Buffer.Foo() would leak Buffer (the C# GC is reluctant
        // to release its refcount on that object)
        virtual ~MediaReaderReadResult();

        property ::Windows::Foundation::TimeSpan Timestamp
        {
            ::Windows::Foundation::TimeSpan get()
            {
                return ::Windows::Foundation::TimeSpan{ _timestamp };
            }
        }

        ///<summary>The data read.</summary>
        property MediaSample^ Sample
        {
            MediaSample^ get()
            {
                return _sample;
            }
        }

        ///<summary>Keep using the sample after MediaReaderReadResult is destroyed.</summary>
        MediaSample^ DetachSample()
        {
            auto sample = _sample;
            _sample = nullptr;
            return sample;
        }

        property bool Error {
            bool get()
            {
                return FAILED(_hr);
            }
        }

        property Platform::String^ ErrorMessage { Platform::String^ get(); }

        property uint32 ErrorCode {
            uint32 get()
            {
                return _hr;
            }
        }

        property bool EndOfStream {
            bool get()
            {
                return !!(MF_SOURCE_READERF_ENDOFSTREAM & _streamFlags);
            }
        }

        property bool NewStream {
            bool get()
            {
                return !!(MF_SOURCE_READERF_NEWSTREAM & _streamFlags);
            }
        };

        property bool CurrentStreamPropertiesChanged {
            bool get()
            {
                return !!(MF_SOURCE_READERF_CURRENTMEDIATYPECHANGED & _streamFlags);
            }
        }

        property bool NativeStreamPropertiesChanged {
            bool get()
            {
                return !!(MF_SOURCE_READERF_NATIVEMEDIATYPECHANGED & _streamFlags);
            }
        }

        property bool Tick {
            bool get()
            {
                return !!(MF_SOURCE_READERF_STREAMTICK & _streamFlags);
            }
        }

        property bool AllEffectsRemoved {
            bool get()
            {
                return !!(MF_SOURCE_READERF_ALLEFFECTSREMOVED & _streamFlags);
            }
        }

    internal:
        MediaReaderReadResult(
            HRESULT hr,
            MF_SOURCE_READER_FLAG streamFlags,
            LONGLONG timestamp,
            _In_opt_ MediaSample^ sample
            )
            : _hr(hr)
            , _streamFlags(streamFlags)
            , _timestamp(timestamp)
            , _sample(sample)
        {
            NT_ASSERT(FAILED(_hr) == ((streamFlags & MF_SOURCE_READERF_ERROR) != 0));
        }

    private:
        const HRESULT _hr;
        const MF_SOURCE_READER_FLAG _streamFlags;
        const LONGLONG _timestamp;
        MediaSample^ _sample;
    };

}

