#ifndef __VIDEOSTREAM_H__
#define __VIDEOSTREAM_H__

#include <thread>
#include "Codec.h"

using namespace std;

class VideoStream {
public:
    thread t;
    VCodec *vcodec;

    VideoStream(VCodec* vc) : vcodec(vc) {}
    virtual ~VideoStream() { delete vcodec; }

    virtual void work(void) = 0;
    virtual void run(void) = 0;
    virtual void stop(void) = 0;
};

#endif
