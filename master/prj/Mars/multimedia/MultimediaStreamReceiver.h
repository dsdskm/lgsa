#ifndef __MULTIMEDIASTREAMRECEIVER_H__
#define __MULTIMEDIASTREAMRECEIVER_H__

#include "RemoteVideoStream.h"
#include "RemoteAudioStream.h"
#include "VideoFrame.h"
#include "AudioChunk.h"
#include "../net/StreamController.h"

class MultimediaStreamAdjustment {
public:
    virtual void adjustScore(int score) = 0;
};

class MultimediaStreamReceiver : public net::StreamReceiverClient {
    int peerIdentifier;
    string peerIpAddress;
    net::StreamController* streamController;
    RemoteVideoStream* remoteVideoStream;
    RemoteAudioStream* remoteAudioStream;
    MultimediaStreamAdjustment *adjustment;
public:
    MultimediaStreamReceiver(string pia, net::StreamController *sc, RemoteVideoStream *rvs, RemoteAudioStream *ras, MultimediaStreamAdjustment *adj);

    virtual void OnReceive(net::StreamType type, const int peer_id, int stream_id, size_t recv_size);

    virtual void OnNetworkScore(int score);

    virtual void setIdentifier(int id) { peerIdentifier = id; }
    void run(void);
    void stop(void);
};
#endif
