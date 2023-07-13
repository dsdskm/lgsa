#ifndef __MJPEGVCODEC_H__
#define __MJPEGVCODEC_H__

#include <vcruntime_string.h>
#include "Codec.h"

using namespace std;

class MjpegVCodec : public VCodec {
    int Q;
public:
    MjpegVCodec();
    ~MjpegVCodec() {}
    virtual bool encode(uint8_t* in, size_t isize, uint8_t* out, size_t* osize);
    virtual bool decode(uint8_t* in, size_t isize, uint8_t* out, size_t* osize);
    virtual bool setInputResolution(int w, int h);
    virtual bool setOutputResolution(int w, int h);
    virtual bool setInputColorSpace(Colorspace cs);
    virtual bool increaseQuality(void);
    virtual bool decreaseQuality(void);
};

#endif

