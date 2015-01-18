#include "pch.h"

using namespace MediaCaptureReader;
using namespace Microsoft::VisualStudio::CppUnitTestFramework;
using namespace Windows::Media::Capture;
using namespace Windows::ApplicationModel::Core;
using namespace Windows::UI::Core;

TEST_CLASS(MediaGraphicsDeviceTests)
{
public:

    TEST_METHOD(CX_WP_MediaGraphicsDevice_Basic)
    {
        Await(CoreApplication::MainView->CoreWindow->Dispatcher->RunAsync(
            CoreDispatcherPriority::Normal,
            ref new DispatchedHandler([]()
        {
            auto capture = ref new MediaCapture();
            Await(capture->InitializeAsync());

            auto device = MediaGraphicsDevice::CreateFromMediaCapture(capture);
            Assert::IsNotNull(device);
        })));
    }

};
