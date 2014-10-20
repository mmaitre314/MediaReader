#pragma once

namespace MediaCaptureReader
{
    public ref class MediaGraphicsDevice sealed
    {
    public:

        static MediaGraphicsDevice^ CreateFromMediaCapture(_In_ WMC::MediaCapture^ capture);

        // IClosable
        virtual ~MediaGraphicsDevice();

    internal:

        MW::ComPtr<IMFDXGIDeviceManager> GetDeviceManager() const
        {
            return _deviceManager;
        }

    private:

        MW::ComPtr<IMFDXGIDeviceManager> _deviceManager;
    };
}
