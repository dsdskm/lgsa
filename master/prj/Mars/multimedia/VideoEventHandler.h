#ifndef __VIDEOEVENTHANDLER_H__
#define __VIDEOEVENTHANDLER_H__

#include <cstdint>

class VideoEventHandler {
public:
    virtual bool onFrameArrive(uint8_t* frame, size_t size, int peerid) = 0;
};

#endif
