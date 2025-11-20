#include <iostream>
#include "utils/about.h"
#include "video/videodevice.h"
#include "threads/iothread.h"

/********** DEBUG SETUP **********/
#define ENABLE_SDEBUG
#define DEBUG_PREFIX "LFCAM"
#include "utils/screenlogger.h"
/*********************************/

#include "Codecs.h"

std::ostream& operator<<(std::ostream& os,LF::video::VideoDeviceInfo& dev)
{
    os << dev.mId << ": " << dev.mName;
    int i = 0;
    for (auto& f : dev.mFormats)
    {
        if (i++)
        {
            os << std::endl;
        }
        os << "\t*" << f;
    }
    return os;
}

void OnNewFrame(LF::video::VideoDevice* dev, std::shared_ptr<LF::graphic::RawDataContainer>, uint32_t dataSize, uint32_t width, uint32_t height, LF::graphic::ColorSpace_t colorspace)
{
    SDEB("Frame: %d bytes, res: %d x %d (%s)", dataSize, width, height, str(colorspace).c_str());
}

int main()
{
    LF::threads::IOThread t;
    t.Start();

    std::cout << "Hello LFCAM: " << About::Version() << std::endl;

    std::vector<LF::video::VideoDeviceInfo> devices = LF::video::VideoDevice::GetDevicesList();
    for (auto& d : devices)
    {
        std::cout << d << std::endl;
    }

    LF::video::VideoDevice* dev = LF::video::VideoDevice::GetVideoDevice(0, 1);
    if (dev)
    {
        CONNECT(dev->NEW_FRAME_AVAILABLE_RAW, OnNewFrame);
        dev->Start();
    }

    t.Join();
}