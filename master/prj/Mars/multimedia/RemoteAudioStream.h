#ifndef __REMOTEAUDIOSTREAM_H__
#define __REMOTEAUDIOSTREAM_H__

#include <string>
#include <list>
#include <mutex>
#include <condition_variable>
#include "Codec.h"
#include "AudioStream.h"
#include "AudioChunk.h"
#include "SpeakerRenderer.h"

class RemoteAudioStream : public AudioStream {
    std::string peerIpAddress;

    std::list<AudioChunk*> queue;
    std::mutex lock;
    std::condition_variable cond;
    AudioChannelQueue* speakerQueue;

    bool terminate;
    virtual void work(void);

public:
    RemoteAudioStream(ACodec* c, std::string ip);
    ~RemoteAudioStream();
    virtual void run(void);
    virtual void stop(void);

    bool put(AudioChunk *ac);
    AudioChunk *get(void);

    std::string getIp(void) { return peerIpAddress; }
};

#endif
