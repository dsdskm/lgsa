#ifndef __REMOTEVIDEOSTREAM_H__
#define __REMOTEVIDEOSTREAM_H__

#include <string>
#include <list>
#include <mutex>
#include <condition_variable>
#include "VideoStream.h"
#include "VideoFrame.h"
#include "BufferedReceiver.h"
#include "Codec.h"

class RemoteVideoStream : public VideoStream {
    int uiId;
    std::string peerIpAddress;
    BufferedReceiver<VideoFrame>* bufferedReceiver;

    std::list<VideoFrame> queue;
    std::mutex lock;
    std::condition_variable cond;

    bool terminate;

    virtual void work(void);
public:
    RemoteVideoStream(VCodec* c, std::string ip);
    RemoteVideoStream(VCodec* c, std::string ip, BufferedReceiver<VideoFrame> *br);
    RemoteVideoStream(VCodec* c, std::string ip, int uiid);
    ~RemoteVideoStream();

    virtual void run(void);
    virtual void stop(void);

    std::string getIp(void) { return peerIpAddress; }
    void setReceiver(BufferedReceiver<VideoFrame>* br);

    bool put(VideoFrame &vf);
    VideoFrame get(void);
};

#endif

