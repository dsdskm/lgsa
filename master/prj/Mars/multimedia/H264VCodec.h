#ifndef __H264VCODEC_H__
#define __H264VCODEC_H__

#include <map>
#include <atomic>

#include "Codec.h"
extern "C" {
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libswscale/swscale.h>
#include <libavutil/imgutils.h>
}

class H264VCodec : public VCodec {
    int width;
    int height;
    int maxBitrate;
    int minBitrate;
    int bitrate;
    int framerate;
    int goplen;
    int maxbframe;

    const AVCodec* enc_avcodec;
    std::atomic<AVCodecContext*> cEnc_avcontext;

    /* Passive Redundancy */
    std::map<int, AVCodecContext*> aEnc_avcontext;

    SwsContext* enc_swsctx;

    const AVCodec* dec_avcodec;
    AVCodecContext* dec_avcontext;
    SwsContext* dec_swsctx;


public:
    H264VCodec();
    virtual bool encode(uint8_t* in, size_t isize, uint8_t* out, size_t* osize);
    virtual bool decode(uint8_t* in, size_t isize, uint8_t* out, size_t* osize);
    virtual bool setInputResolution(int w, int h);
    virtual bool setOutputResolution(int w, int h);
    virtual bool setInputColorSpace(Colorspace cs);
    virtual bool increaseQuality(void);
    virtual bool decreaseQuality(void);
};

#endif
