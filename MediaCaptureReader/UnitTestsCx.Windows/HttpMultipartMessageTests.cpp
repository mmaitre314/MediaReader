#include "pch.h"
#include "..\MediaCaptureReader\MediaCaptureReader.Shared\HttpMultipartMessage.h"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;
using namespace Platform;
using namespace std;
using namespace Windows::Storage::Streams;

#define STRING_AND_SIZE(str) ((unsigned char*)str), (ARRAYSIZE(str)-1)

TEST_CLASS(HttpMultipartMessageTests)
{
public:

    //
    // Windows tests use NullMediaCapture as the real MediaCapture tries to pop up a consent UI which is nowhere to be seen
    // and cannot be automatically dismissed from within the tests
    //

    TEST_METHOD(CX_W_HttpMultipartMessage_ShortBoundary)
    {
        HttpMultipartMessage msg;

        msg.SetBoundary(L"b");
        Assert::IsNull(msg.GetPart());
        
        msg.Append(CreateBuffer(STRING_AND_SIZE("\r\n--b\r\n")));
        msg.Append(CreateBuffer(STRING_AND_SIZE("\r\n")));
        msg.Append(CreateBuffer(STRING_AND_SIZE("message")));
        msg.Append(CreateBuffer(STRING_AND_SIZE("\r\n--b\r\n")));

        IBuffer^ part = msg.GetPart();
        Assert::IsNotNull(part);
        Assert::AreEqual(ARRAYSIZE("message") - 1, part->Length);
        Assert::AreEqual(0, memcmp(GetData(part), STRING_AND_SIZE("message")));

        Assert::IsNull(msg.GetPart());

        msg.Append(CreateBuffer(STRING_AND_SIZE("\r\n")));
        msg.Append(CreateBuffer(STRING_AND_SIZE("MESSAGE")));
        msg.Append(CreateBuffer(STRING_AND_SIZE("\r\n--b\r\n")));

        part = msg.GetPart();
        Assert::IsNotNull(part);
        Assert::AreEqual(sizeof("MESSAGE") - 1, part->Length);
        Assert::AreEqual(0, memcmp(GetData(part), STRING_AND_SIZE("MESSAGE")));
    }

    TEST_METHOD(CX_W_HMM_ShortBoundaryShortAppend)
    {
        HttpMultipartMessage msg;

        msg.SetBoundary(L"b");
        Assert::IsNull(msg.GetPart());

        msg.Append(CreateBuffer(STRING_AND_SIZE("\r")));
        msg.Append(CreateBuffer(STRING_AND_SIZE("\n")));
        msg.Append(CreateBuffer(STRING_AND_SIZE("-")));
        msg.Append(CreateBuffer(STRING_AND_SIZE("-")));
        msg.Append(CreateBuffer(STRING_AND_SIZE("b")));
        msg.Append(CreateBuffer(STRING_AND_SIZE("\r")));
        msg.Append(CreateBuffer(STRING_AND_SIZE("\n")));
        msg.Append(CreateBuffer(STRING_AND_SIZE("\r")));
        msg.Append(CreateBuffer(STRING_AND_SIZE("\n")));
        msg.Append(CreateBuffer(STRING_AND_SIZE("m")));
        msg.Append(CreateBuffer(STRING_AND_SIZE("s")));
        msg.Append(CreateBuffer(STRING_AND_SIZE("g")));
        msg.Append(CreateBuffer(STRING_AND_SIZE("\r")));
        msg.Append(CreateBuffer(STRING_AND_SIZE("\n")));
        msg.Append(CreateBuffer(STRING_AND_SIZE("-")));
        msg.Append(CreateBuffer(STRING_AND_SIZE("-")));
        msg.Append(CreateBuffer(STRING_AND_SIZE("b")));
        msg.Append(CreateBuffer(STRING_AND_SIZE("\r")));
        msg.Append(CreateBuffer(STRING_AND_SIZE("\n")));

        IBuffer^ part = msg.GetPart();
        Assert::IsNotNull(part);
        Assert::AreEqual(ARRAYSIZE("msg") - 1, part->Length);
        Assert::AreEqual(0, memcmp(GetData(part), STRING_AND_SIZE("msg")));

        Assert::IsNull(msg.GetPart());

        msg.Append(CreateBuffer(STRING_AND_SIZE("\r")));
        msg.Append(CreateBuffer(STRING_AND_SIZE("\n")));
        msg.Append(CreateBuffer(STRING_AND_SIZE("M")));
        msg.Append(CreateBuffer(STRING_AND_SIZE("S")));
        msg.Append(CreateBuffer(STRING_AND_SIZE("G")));
        msg.Append(CreateBuffer(STRING_AND_SIZE("\r")));
        msg.Append(CreateBuffer(STRING_AND_SIZE("\n")));
        msg.Append(CreateBuffer(STRING_AND_SIZE("-")));
        msg.Append(CreateBuffer(STRING_AND_SIZE("-")));
        msg.Append(CreateBuffer(STRING_AND_SIZE("b")));
        msg.Append(CreateBuffer(STRING_AND_SIZE("\r")));
        msg.Append(CreateBuffer(STRING_AND_SIZE("\n")));

        part = msg.GetPart();
        Assert::IsNotNull(part);
        Assert::AreEqual(sizeof("MSG") - 1, part->Length);
        Assert::AreEqual(0, memcmp(GetData(part), STRING_AND_SIZE("MSG")));
    }

    TEST_METHOD(CX_W_HttpMultipartMessage_MyBoundaryHyphenIssue)
    {
        HttpMultipartMessage msg;

        msg.SetBoundary(L"--myboundary");
        Assert::IsNull(msg.GetPart());

        msg.Append(CreateBuffer(STRING_AND_SIZE("\r\n--myboundary\r\n")));
        msg.Append(CreateBuffer(STRING_AND_SIZE("\r\n")));
        msg.Append(CreateBuffer(STRING_AND_SIZE("message")));
        msg.Append(CreateBuffer(STRING_AND_SIZE("\r\n--myboundary\r\n")));

        IBuffer^ part = msg.GetPart();
        Assert::IsNotNull(part);
        Assert::AreEqual(ARRAYSIZE("message") - 1, part->Length);
        Assert::AreEqual(0, memcmp(GetData(part), STRING_AND_SIZE("message")));

        Assert::IsNull(msg.GetPart());

        msg.Append(CreateBuffer(STRING_AND_SIZE("\r\n")));
        msg.Append(CreateBuffer(STRING_AND_SIZE("MESSAGE")));
        msg.Append(CreateBuffer(STRING_AND_SIZE("\r\n--myboundary\r\n")));

        part = msg.GetPart();
        Assert::IsNotNull(part);
        Assert::AreEqual(sizeof("MESSAGE") - 1, part->Length);
        Assert::AreEqual(0, memcmp(GetData(part), STRING_AND_SIZE("MESSAGE")));
    }

};
