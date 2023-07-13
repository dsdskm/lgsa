#ifndef __AUDIOSTREAM_H__
#define __AUDIOSTREAM_H__

#include <thread>
#include "Codec.h"

using namespace std;

class AudioStream
{
public:
    thread t;
    ACodec* acodec;

    AudioStream(ACodec* ac) : acodec(ac) {}
    virtual ~AudioStream() {}

    virtual void work(void) = 0;
    virtual void run(void) = 0;
    virtual void stop() = 0;
};

#endif

