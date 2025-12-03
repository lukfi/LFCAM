#pragma once

#include "video/videodevice.h"
#include "threads/iothread.h"
#include <string>
#include <mutex>
#include "UniDecoder.h"
#include "imagewindow.h"

class VideoController
{
public:
    VideoController(
#ifdef WCV_INPLACE
        ImageWindow& win
#endif
        );
    std::string GetAllDevicesInfo() const;
    bool CurrentlyRunning(int& deviceId, int& formatId) const;

    bool StopDevice();
    bool StartDevice(uint32_t devId, uint32_t formatId);

#ifdef WCV_INPLACE
    ImageWindow& mWin;
#endif

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

    LF::threads::IOThread mThread;

    int mRunningDeviceID{ -1 };
    int mRunningFormatID{ -1 };
};

