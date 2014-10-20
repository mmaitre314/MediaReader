using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using Microsoft.VisualStudio.TestPlatform.UnitTestFramework;
using MediaCaptureReader;

namespace UnitTests.NuGet.Windows
{
    [TestClass]
    public class UnitTests
    {
        [TestMethod]
        public void CS_W_N_UseCaptureStreamType()
        {
            Assert.AreEqual(0, CaptureStreamType.Preview);
        }
    }
}
