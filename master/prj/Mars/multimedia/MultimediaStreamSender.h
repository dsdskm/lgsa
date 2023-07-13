#ifndef __MULTIMEDIASTREAMSENDER_H__
#define __MULTIMEDIASTREAMSENDER_H__

#include "../net/StreamController.h"
#include "VideoFrame.h"
#include "AudioChunk.h"

using namespace std;
using namespace net;

class MultimediaStreamSender {
public:
    StreamController *streamController;
    virtual int send(uint8_t* buf, size_t len) = 0;

    MultimediaStreamSender(StreamController *sc): streamController(sc) {}
    virtual ~MultimediaStreamSender() {}
};

class AudioStreamSender : public MultimediaStreamSender {
    virtual int send(uint8_t* buf, size_t len) {
        return streamController->Send(STREAM_AUDIO, (char*)buf, len);
    }
public:
    AudioStreamSender(StreamController *sc): MultimediaStreamSender(sc) {}

    int put(AudioChunk &ac) {
        return send((uint8_t*)ac.data, ac.NumberOfPayloadByte);
    }
};

class VideoStreamSender : public MultimediaStreamSender {
    virtual int send(uint8_t* buf, size_t len) {
        return streamController->Send(STREAM_VIDEO, (char*)buf, len);
    }
public:
    VideoStreamSender(StreamController *sc): MultimediaStreamSender(sc) {}

    int put(VideoFrame &vf) {
        return send(vf.data, vf.bytesUsed);
    }

    int put(uint8_t *buf, size_t len) {
        return send(buf, len);
    }
};
#endif
