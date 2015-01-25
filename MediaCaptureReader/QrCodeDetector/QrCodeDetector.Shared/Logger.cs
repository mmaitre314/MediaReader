using System;
using System.Collections.Generic;
using System.Diagnostics.Tracing;
using System.Text;

namespace QrCodeDetector
{
    [EventSource(Name = "MMaitre-QrCodeDetector")]
    sealed class Logger : EventSource
    {
        public static Logger Events = new Logger();

        // Unstructured traces

        [NonEvent]
        public static void Write(string message)
        {
            Events.Message(message);
        }

        [NonEvent]
        public static void Write(string format, params object[] args)
        {
            if (Events.IsEnabled())
            {
                Events.Message(String.Format(format, args));
            }
        }

        [Event(1, Level = EventLevel.Verbose)]
        private void Message(string message) { Events.WriteEvent(1, message); }

        // Performance markers

        [Event(2)]
        public void VideoBarcodeReader_SampleDropped() { Events.WriteEvent(2); }

        public class Tasks
        {
            public const EventTask VideoBarcodeReader_DecodeSample = (EventTask)1;
            public const EventTask VideoStream_Read = (EventTask)2;
        }

        [Event(3, Task = Tasks.VideoBarcodeReader_DecodeSample, Opcode = EventOpcode.Start)]
        public void VideoBarcodeReader_DecodeSampleStart() { Events.WriteEvent(3); }

        [Event(4, Task = Tasks.VideoBarcodeReader_DecodeSample, Opcode = EventOpcode.Stop)]
        public void VideoBarcodeReader_DecodeSampleStop(string result) { Events.WriteEvent(4, result); }

        [Event(5, Task = Tasks.VideoStream_Read, Opcode = EventOpcode.Start)]
        public void VideoStream_ReadStart() { Events.WriteEvent(5); }

        [Event(6, Task = Tasks.VideoStream_Read, Opcode = EventOpcode.Stop)]
        public void VideoStream_ReadStop() { Events.WriteEvent(6); }
    }
}
