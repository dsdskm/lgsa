#ifndef __CODEC_H__
#define __CODEC_H__
#include <cstdint>

class Codec {
public:
    virtual bool encode(uint8_t* in, size_t isize, uint8_t *out, size_t *osize) = 0;
    virtual bool decode(uint8_t* in, size_t isize, uint8_t *out, size_t *osize) = 0;
};

class VCodec: public Codec {
public:
    enum Colorspace
    {
        COLORSPACE_RGB,
        COLORSPACE_YUC,
    };

    virtual bool setInputResolution(int w, int h) = 0;
    virtual bool setOutputResolution(int w, int h) = 0;
    virtual bool setInputColorSpace(Colorspace cs) = 0;
    virtual bool increaseQuality(void) { return false; }
    virtual bool decreaseQuality(void) { return false; }
};

class ACodec: public Codec {
public:
    virtual bool setInputChannel(int n) = 0;
    virtual bool setInputBitrate(int r) = 0;
    virtual bool setOutputBitrate(int r) = 0;
};
#endif
