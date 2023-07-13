#ifndef __VIDEOFRAME_H__
#define __VIDEOFRAME_H__

#include <cstdint>

// TODO: make perfect forward
class VideoFrame {
public:
    int width;
    int height;
    int bpp;
    uint8_t* data;
    size_t bytesUsed;

    VideoFrame() : width(0), height(0), bpp(0), bytesUsed(0), data(nullptr) {}

    VideoFrame(int w, int h, int b, uint8_t *d, size_t s) : width(w), height(h), bpp(b), bytesUsed(s), data(d) {}

    ~VideoFrame() { /*delete[] data;*/ }
};
#endif
