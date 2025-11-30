#include <iostream>
#include "utils/about.h"
#include "video/videodevice.h"
#include "threads/iothread.h"
#include "utils/stringutils.h"

#include "fs/file.h"
#include "VideoController.h"
#include "RequestHandler.h"

#include "UniDecoder.h"

/********** DEBUG SETUP **********/
#define ENABLE_SDEBUG
#define DEBUG_PREFIX "LFCAM"
#include "utils/screenlogger.h"
/*********************************/

bool SaveRawImageToFile(LF::graphic::RawImage& ri, std::string path)
{
    LF::fs::File f(path);
    f.CreateOpen();
    return f.Put(ri.GetRawData(), ri.GetDataSize()) == ri.GetDataSize();
}

std::ostream& operator<<(std::ostream& os,LF::video::VideoDeviceInfo& dev)
{
    os << dev.mId << ": " << dev.mName;
    int i = 0;
    for (auto& f : dev.mFormats)
    {

        os << std::endl;
        os << "\t" << i++ << " * " << f;
    }
    return os;
}

shared_ptr<UniDecoder> gDecoder;

void OnNewFrame(LF::video::VideoDevice* dev, std::shared_ptr<LF::graphic::RawDataContainer> data, uint32_t dataSize, uint32_t width, uint32_t height, LF::graphic::ColorSpace_t colorspace)
{
    bool dropFrame = (gDecoder == nullptr);

    if (!dropFrame && colorspace == LF::graphic::ColorSpace_t::H264)
    {
        static bool haveSPS = false;
        static bool havePPS = false;
        static bool haveIDR = false;
        // Scan for SPS and PPS
        for (DWORD i = 0; i + 4 < dataSize; i++)
        {
            if (data->Get()[i] == 0 && data->Get()[i + 1] == 0 && data->Get()[i + 2] == 1)
            {
                uint8_t nalType = data->Get()[i + 3] & 0x1F;
                if (nalType == 7) haveSPS = true;
                if (nalType == 8) havePPS = true;
                if (nalType == 5) haveIDR = true;
            }
        }

        if (!haveSPS || !havePPS)
        {
            dropFrame = true;
        }
    }

    if (!dropFrame)
    {
        //SDEB("Feed Frame: %d bytes, res: %d x %d (%s)", dataSize, width, height, str(colorspace).c_str());
        gDecoder->FeedRawData(data->Get(), data->GetSize());
    }
    else
    {
        SDEB("Drop frame: decoder: %p, colorspace: %s", gDecoder.get(), str(colorspace).c_str());
    }
}

RequestHandler gHandler;

int main()
{
    VideoController gController;
    RequestHandler gHandler(gController);
    BasicWebServer gServer(8080, BasicWebServer::NO_Server, &gHandler);

    //CONNECT(gWCV.CONNECTED, OnWCVConnected);
    //gWCV.Start(true, true);
    LF::threads::IOThread t;
    t.Start();

    std::cout << "Hello LFCAM: " << About::Version() << std::endl;

    //gDev = LF::video::VideoDevice::GetVideoDevice(0, 1);
    //gDev = LF::video::VideoDevice::GetVideoDevice(1, 12); // CAM 1080p NV12
    //gDev = LF::video::VideoDevice::GetVideoDevice(1, 13); // CAM 1080p MJPG
    //gDev = LF::video::VideoDevice::GetVideoDevice(1, 40); // CAM 1080p  YUYV

    //if (gDev)
    //{
    //    gDecoder = UniDecoder::Get(gDev->GetConfiguredFormat().colorspace, gDev->GetConfiguredFormat().resolution.width, gDev->GetConfiguredFormat().resolution.height);
    //    CONNECT(gDev->NEW_FRAME_AVAILABLE_RAW, OnNewFrame);
    //    gDev->Start();
    //}

    t.Join();
}