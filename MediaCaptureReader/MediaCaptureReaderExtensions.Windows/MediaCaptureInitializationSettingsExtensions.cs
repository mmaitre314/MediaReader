using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using Windows.Devices.Enumeration;
using Windows.Media.Capture;

namespace MediaCaptureReaderExtensions
{
    /// <summary></summary>
    public enum VideoDeviceSelection
    {
        /// <summary></summary>
        BackOrFirst,

        /// <summary></summary>
        FrontOrFirst
    }

    /// <summary>
    /// Extension methods on MediaCaptureInitializationSettings
    /// </summary>
    static public class MediaCaptureInitializationSettingsExtensions
    {
        /// <summary>
        /// Enumerate video devices and set MediaCaptureInitializationSettings.VideoDeviceId based on the selection criteria.
        /// </summary>
        /// <returns>True if a video device matching the selection criteria was found.</returns>
        public static async Task<bool> SelectVideoDeviceAsync(this MediaCaptureInitializationSettings settings, VideoDeviceSelection selection)
        {
            var devices = await DeviceInformation.FindAllAsync(DeviceClass.VideoCapture);

            if (devices.Count == 0)
            {
                return false;
            }

            // Select front/back if present
            foreach (var device in devices)
            {
                if (device.EnclosureLocation == null)
                {
                    continue;
                }

                switch (selection)
                {
                    case VideoDeviceSelection.BackOrFirst:
                        if (device.EnclosureLocation.Panel == Panel.Back)
                        {
                            settings.VideoDeviceId = device.Id;
                            return true;
                        }
                        break;

                    case VideoDeviceSelection.FrontOrFirst:
                        if (device.EnclosureLocation.Panel == Panel.Front)
                        {
                            settings.VideoDeviceId = device.Id;
                            return true;
                        }
                        break;
                }
            }

            // Select first if no front/back
            settings.VideoDeviceId = devices[0].Id;
            return true;
        }
    }
}
