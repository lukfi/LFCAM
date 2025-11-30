#pragma once
#include "Codecs.h"
#include "H264/H264Decoder/H264Decoder.h"

#include "utils/screenlogger.h"

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
        SDEB("Creating UniDecoder for input format: %s", str(inputColorspace).c_str());

        if (inputColorspace == LF::graphic::ColorSpace_t::H264)
        {
            shared_ptr<UniDecoder> ret(new UniDecoder(LF::graphic::ColorSpace_t::H264));
            ret->Configure(width, height, LF::graphic::ColorSpace_t::NV12);
            return ret;
        }
        else if (inputColorspace == LF::graphic::ColorSpace_t::NV12 ||
            inputColorspace == LF::graphic::ColorSpace_t::YUYV422)
        {
            shared_ptr<UniDecoder> ret(new UniDecoder(inputColorspace));
            ret->Configure(width, height, inputColorspace);
            return ret;
        }
        else if (inputColorspace == LF::graphic::ColorSpace_t::MJPG)
        {
            shared_ptr<UniDecoder> ret(new UniDecoder(LF::graphic::ColorSpace_t::MJPG));
            ret->Configure(width, height, LF::graphic::ColorSpace_t::MJPG);
            return ret;
        }
        else
        {
            SERR("Can't create a decoder for format: %s", str(inputColorspace).c_str());
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
        else if (mInputColorspace == LF::graphic::ColorSpace_t::NV12 ||
                 mInputColorspace == LF::graphic::ColorSpace_t::YUYV422)
        {
            std::shared_ptr<LF::graphic::RawImage> ri = std::make_shared<LF::graphic::RawImage>(mWidth, mHeight, mInputColorspace, data);
            OnFrame(ri);
            ret = true;
        }
        if (mInputColorspace == LF::graphic::ColorSpace_t::MJPG)
        {
            std::shared_ptr<LF::graphic::RawImage> ri = std::make_shared<LF::graphic::RawImage>(mWidth, mHeight, LF::graphic::ColorSpace_t::MJPG, data, dataSize);
            OnFrame(ri);
            ret = true;
        }

        if (!ret)
        {
            SWARN("Failed to feed the decoder");
        }
        return ret;
    }
    
    virtual void OnFrame(std::shared_ptr<LF::graphic::RawImage> ri)
    {
        if (ri->GetWidth() > mWidth || ri->GetHeight() > mHeight)
        {
            ri->SetRealResolution(mWidth, mHeight);
        }
        SINFO("Got image %d x %d", ri.GetWidth(), ri.GetHeight());
    }

protected:
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

    virtual void OnNewFrame(std::shared_ptr<LF::graphic::RawDataContainer> data, uint32_t dataSize, uint32_t width, uint32_t height) override {}

    LF::codecs::VideoDecoder* mVideoDecoder{ nullptr };

    LF::graphic::ColorSpace_t mOutputColorspace{ LF::graphic::ColorSpace_t::NONE };
    LF::graphic::ColorSpace_t mInputColorspace{ LF::graphic::ColorSpace_t::NONE };
    uint32_t mWidth{ 0 };
    uint32_t mHeight{ 0 };
};
