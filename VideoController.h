#pragma once

#include "video/videodevice.h"
#include <string>
#include <mutex>
#include "UniDecoder.h"

class VideoController
{
public:
    VideoController();
    std::string GetAllDevicesInfo() const;
    bool CurrentlyRunning(int& deviceId, int& formatId) const;

    bool StopDevice();
    bool StartDevice(uint32_t devId, uint32_t formatId);

private:
    void OnNewFrame(LF::video::VideoDevice* dev, std::shared_ptr<LF::graphic::RawDataContainer> data, uint32_t dataSize, uint32_t width, uint32_t height, LF::graphic::ColorSpace_t colorspace);

    class MyUniDecoder : public UniDecoder
    {
    public:
        static shared_ptr<MyUniDecoder> Get(LF::graphic::ColorSpace_t inputColorspace, uint32_t width, uint32_t height);

        virtual void OnFrame(std::shared_ptr<LF::graphic::RawImage> ri) override;

    protected:
        MyUniDecoder(LF::graphic::ColorSpace_t inputColorspace) : UniDecoder(inputColorspace) {}
    };

    std::mutex mDeviceMutex;
    std::shared_ptr<LF::video::VideoDevice> mDevice;
    shared_ptr<UniDecoder> mDecoder;

    int mRunningDeviceID{ -1 };
    int mRunningFormatID{ -1 };
};

