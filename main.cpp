#include <iostream>
#include "utils/about.h"
#include "video/videodevice.h"
#include "threads/iothread.h"
#include "utils/stringutils.h"

#include "fs/file.h"

/********** DEBUG SETUP **********/
#define ENABLE_SDEBUG
#define DEBUG_PREFIX "LFCAM"
#include "utils/screenlogger.h"
/*********************************/

#include "Codecs.h"
#include "H264/H264Decoder/H264Decoder.h"

LF::video::VideoDevice* gDev = nullptr;

#include "../Common/Utils/WillowCmdVideo/wcvclientlite.h"
WCVClient gWCV("D:/workspace/Common/Utils/WillowCmdVideo/x64/Release/WillowCmdVideo.exe");

bool SaveRawImageToFile(LF::graphic::RawImage& ri, std::string path)
{
    LF::fs::File f(path);
    f.CreateOpen();
    return f.Put(ri.GetRawData(), ri.GetDataSize()) == ri.GetDataSize();
}

class UniDecoder : protected LF::codecs::VideoDecoder
{
public:
    ~UniDecoder()
    {
        if (mVideoDecoder)
        {
            delete mVideoDecoder;
            mVideoDecoder = nullptr;
        }
    }
    static shared_ptr<UniDecoder> Get(LF::graphic::ColorSpace_t inputColorspace, uint32_t width, uint32_t height)
    {
        if (inputColorspace == LF::graphic::ColorSpace_t::H264)
        {
            shared_ptr<UniDecoder> ret(new UniDecoder(LF::graphic::ColorSpace_t::H264));
            ret->Configure(width, height, LF::graphic::ColorSpace_t::NV12);
            return ret;
        }
        else if (inputColorspace == LF::graphic::ColorSpace_t::NV12)
        {
            shared_ptr<UniDecoder> ret(new UniDecoder(LF::graphic::ColorSpace_t::NV12));
            ret->Configure(width, height, LF::graphic::ColorSpace_t::NV12);
            return ret;
        }
        else if (inputColorspace == LF::graphic::ColorSpace_t::MJPG)
        {
            shared_ptr<UniDecoder> ret(new UniDecoder(LF::graphic::ColorSpace_t::MJPG));
            ret->Configure(width, height, LF::graphic::ColorSpace_t::MJPG);
            return ret;
        }
        return nullptr;
    }

    virtual bool FeedRawData(const uint8_t* data, uint32_t dataSize) override
    {
        bool ret = false;
        if (mVideoDecoder)
        {
            ret = mVideoDecoder->FeedRawData(data, dataSize);
        }
        else if (mInputColorspace == LF::graphic::ColorSpace_t::NV12)
        {
            LF::graphic::RawImage ri(mWidth, mHeight, LF::graphic::ColorSpace_t::NV12, data);
            OnFrame(ri);
            ret = true;
        }
        if (mInputColorspace == LF::graphic::ColorSpace_t::MJPG)
        {
            LF::graphic::RawImage ri(mWidth, mHeight, LF::graphic::ColorSpace_t::MJPG, data, dataSize);
            OnFrame(ri);
            ret = true;
        }

        if (!ret)
        {
            SWARN("Failed to feed the decoder");
        }
        return ret;
    }

private:
    UniDecoder(LF::graphic::ColorSpace_t inputColorspace) : 
        mInputColorspace(inputColorspace)
    {
        if (inputColorspace == LF::graphic::ColorSpace_t::H264)
        {
            mVideoDecoder = new LF::codecs::H264::H264Decoder();
            CONNECT(mVideoDecoder->FRAME_AVAILABLE, UniDecoder, OnFrame);
        }
    }

    virtual bool Configure(uint32_t inputWidth, uint32_t inputHeight, LF::graphic::ColorSpace_t outputColorspace, uint32_t outputWidth = 0, uint32_t outputHeight = 0) override
    {
        mWidth = inputWidth;
        mHeight = inputHeight;
        mOutputColorspace = outputColorspace;

        if (mVideoDecoder)
        {
            return mVideoDecoder->Configure(mWidth, mHeight, mOutputColorspace);
        }
        return true;
    }

    void OnFrame(LF::graphic::RawImage& ri)
    {
        SINFO("Got image %d x %d", ri.GetWidth(), ri.GetHeight());
        if (ri.GetWidth() > mWidth || ri.GetHeight() > mHeight)
        {
            ri.SetRealResolution(mWidth, mHeight);
        }

        //ri.ConvertToRGB24();
        gWCV.ShowImage(ri);
        //gDev->Stop();

        static int i = 0;
        if (i++ == 0)
        {
            SaveRawImageToFile(ri, "D:/image");
        }

    }

    virtual void OnNewFrame(std::shared_ptr<LF::graphic::RawDataContainer> data, uint32_t dataSize, uint32_t width, uint32_t height) override {}

    LF::codecs::VideoDecoder* mVideoDecoder{ nullptr };

    LF::graphic::ColorSpace_t mOutputColorspace{ LF::graphic::ColorSpace_t::NONE };
    LF::graphic::ColorSpace_t mInputColorspace{ LF::graphic::ColorSpace_t ::NONE};
    uint32_t mWidth{0};
    uint32_t mHeight{0};
};

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

int main()
{
    gWCV.Start(true, true);
    gWCV.ShowImage("D:/workspace/Common/Utils/WillowCmdVideo/graphics/screen.jpg");
    LF::threads::IOThread t;
    t.Start();

    std::cout << "Hello LFCAM: " << About::Version() << std::endl;

    std::vector<LF::video::VideoDeviceInfo> devices = LF::video::VideoDevice::GetDevicesList();
    for (auto& d : devices)
    {
        std::cout << d << std::endl;
    }

    //gDev = LF::video::VideoDevice::GetVideoDevice(0, 1);
    //gDev = LF::video::VideoDevice::GetVideoDevice(1, 12); // CAM 1080p NV12
    gDev = LF::video::VideoDevice::GetVideoDevice(1, 13); // CAM 1080p MJPG

    if (gDev)
    {
        gDecoder = UniDecoder::Get(gDev->GetConfiguredFormat().colorspace, gDev->GetConfiguredFormat().resolution.width, gDev->GetConfiguredFormat().resolution.height);
        CONNECT(gDev->NEW_FRAME_AVAILABLE_RAW, OnNewFrame);
        gDev->Start();
    }

    t.Join();
}