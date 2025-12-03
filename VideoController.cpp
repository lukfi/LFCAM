#include "VideoController.h"
#include "utils/json.h"

#ifndef WCV_INPLACE
#include "../Common/Utils/WillowCmdVideo/wcvclientlite.h"
WCVClient gWCV("D:/workspace/Common/Utils/WillowCmdVideo/x64/Release/WillowCmdVideo.exe");

void OnWCVConnected()
{
    SINFO("WCV Connected");
    gWCV.ShowImage("D:/workspace/Common/Utils/WillowCmdVideo/graphics/screen.jpg");
    //gC->StartDevice(0, 1);
}
#endif

VideoController* gC = nullptr;

/********** DEBUG SETUP **********/
#define ENABLE_SDEBUG
#define DEBUG_PREFIX "VideoController"
#include "utils/screenlogger.h"
/*********************************/

std::shared_ptr<VideoController::MyUniDecoder> VideoController::MyUniDecoder::Get(LF::graphic::ColorSpace_t inputColorspace, uint32_t width, uint32_t height)
{
    SDEB("Creating UniDecoder for input format: %s", str(inputColorspace).c_str());

    if (inputColorspace == LF::graphic::ColorSpace_t::H264)
    {
        shared_ptr<MyUniDecoder> ret(new MyUniDecoder(LF::graphic::ColorSpace_t::H264));
        ret->Configure(width, height, LF::graphic::ColorSpace_t::NV12);
        return ret;
    }
    else if (inputColorspace == LF::graphic::ColorSpace_t::NV12 ||
        inputColorspace == LF::graphic::ColorSpace_t::YUYV422)
    {
        shared_ptr<MyUniDecoder> ret(new MyUniDecoder(inputColorspace));
        ret->Configure(width, height, inputColorspace);
        return ret;
    }
    else if (inputColorspace == LF::graphic::ColorSpace_t::MJPG)
    {
        shared_ptr<MyUniDecoder> ret(new MyUniDecoder(LF::graphic::ColorSpace_t::MJPG));
        ret->Configure(width, height, LF::graphic::ColorSpace_t::MJPG);
        return ret;
    }
    else
    {
        SERR("Can't create a decoder for format: %s", str(inputColorspace).c_str());
    }
    return nullptr;
}

void VideoController::MyUniDecoder::OnFrame(std::shared_ptr<LF::graphic::RawImage> ri)
{
    if (ri->GetWidth() > mWidth || ri->GetHeight() > mHeight)
    {
        ri->SetRealResolution(mWidth, mHeight);
    }
    SINFO("Got image %d x %d", ri.GetWidth(), ri.GetHeight());

    gWCV.ShowImage(*ri);
}

VideoController::VideoController()
{
    gC = this;
    CONNECT(gWCV.CONNECTED, OnWCVConnected);
    gWCV.Start(true);
}

std::string VideoController::GetAllDevicesInfo() const
{
    std::vector<LF::video::VideoDeviceInfo> devices = LF::video::VideoDevice::GetDevicesList();

    json::JSON devsJson = json::Array();
    for (auto& d : devices)
    {
        json::JSON obj = json::Object();
        obj["id"] = d.mId;
        obj["name"] = d.mName;
        obj["internal"] = d.mInternalName;
        obj["formats"] = json::Array();

        int i = 0;
        for (auto& f : d.mFormats)
        {
            json::JSON fmt = json::Object();
            fmt["id"] = i;
            fmt["colorspace"] = str(f.colorspace);
            fmt["info"] = f.additionalInfo;
            fmt["width"] = f.resolution.width;
            fmt["height"] = f.resolution.height;
            ++i;

            obj["formats"].append(fmt);
        }
        devsJson.append(obj);
    }
    return devsJson.dump();
}

bool VideoController::CurrentlyRunning(int& deviceId, int& formatId) const
{
    if (mRunningDeviceID > -1 && mRunningFormatID > -1)
    {
        deviceId = mRunningDeviceID;
        formatId = mRunningFormatID;
        return true;
    }
    return false;
}

void VideoController::OnNewFrame(LF::video::VideoDevice* dev, std::shared_ptr<LF::graphic::RawDataContainer> data, uint32_t dataSize, uint32_t width, uint32_t height, LF::graphic::ColorSpace_t colorspace)
{
    mDecoder->FeedRawData(data->Get(), data->GetSize());
}

bool VideoController::StopDevice()
{
    bool ret = false;
    std::lock_guard<std::mutex> lock(mDeviceMutex);
    if (mDevice)
    {
        mDevice->Stop();
        mDevice = nullptr;
        mDecoder = nullptr;
        ret = true;

        mRunningDeviceID = -1;
        mRunningFormatID = -1;
    }
    return ret;
}

bool VideoController::StartDevice(uint32_t devId, uint32_t formatId)
{
    bool ret = false;
    std::lock_guard<std::mutex> lock(mDeviceMutex);
    if (!mDevice)
    {
        mDevice = std::shared_ptr<LF::video::VideoDevice>(LF::video::VideoDevice::GetVideoDevice(devId, formatId));
        if (mDevice)
        {
            mDecoder = MyUniDecoder::Get(mDevice->GetConfiguredFormat().colorspace, mDevice->GetConfiguredFormat().resolution.width, mDevice->GetConfiguredFormat().resolution.height);
            CONNECT(mDevice->NEW_FRAME_AVAILABLE_RAW, VideoController, OnNewFrame);
            if (mDevice->Start())
            {
                mRunningDeviceID = devId;
                mRunningFormatID = formatId;
            }
        }
    }
    return ret;
}
