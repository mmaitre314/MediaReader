#pragma once

namespace MediaCaptureReader
{
    public ref class MediaGraphicsDevice sealed
    {
    public:

        static MediaGraphicsDevice^ CreateFromMediaCapture(_In_ WMC::MediaCapture^ capture);

        MediaGraphicsDevice();

        // IClosable
        virtual ~MediaGraphicsDevice();

    internal:

        MediaGraphicsDevice(_In_ const MW::ComPtr<IMFDXGIDeviceManager>& deviceManager);

        MW::ComPtr<IMFDXGIDeviceManager> GetDeviceManager() const
        {
            return _deviceManager;
        }

    private:

        MW::ComPtr<IMFDXGIDeviceManager> _deviceManager;
        unsigned int _deviceResetToken; // Non-null if device owned by this object
    };
}
