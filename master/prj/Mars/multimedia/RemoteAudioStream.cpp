#include <iostream>
#include <chrono>
#include "RemoteAudioStream.h"
#include "MultimediaManager.h"
#include "../MarsLog.h"
#include "OPUSACodec.h"

using namespace std;

RemoteAudioStream::RemoteAudioStream(ACodec* ac, string ip)
    : peerIpAddress(ip), speakerQueue(nullptr), terminate(false), AudioStream(ac)
{
    speakerQueue = MARSMultimediaManager.getSpeaker()->registerProducer();
    cout << "[Remote Audio Stream " << peerIpAddress << " ] Create\n";
}

RemoteAudioStream::~RemoteAudioStream()
{
    cout << "[Remote Audio Stream " << peerIpAddress << " ] Destroy\n";
}

bool RemoteAudioStream::put(AudioChunk *ac)
{
    unique_lock<mutex> uniqueLock(lock);
    queue.push_back(ac);
    cond.notify_one();
    return true;
}

AudioChunk *RemoteAudioStream::get(void)
{
    unique_lock<mutex> uniqueLock(lock);

    while (!terminate && queue.empty())
    {
        int centisecs = speakerQueue->getThreshold();

        if (!cond.wait_for(uniqueLock, chrono::milliseconds(centisecs*10),
                    [this]() { return terminate || !queue.empty(); }))
        {
            ALOG_D("[RemoveAudioStream %s] packet receive wait timeout\n", peerIpAddress.c_str());

            if (!speakerQueue->getFlow() && !speakerQueue->IsEmpty())
                speakerQueue->flowOn();
        }
    }

    if (terminate)
        return nullptr;

    AudioChunk *ac = queue.front();
    queue.pop_front();

    return ac;
}

void RemoteAudioStream::work(void)
{
    while (!terminate)
    {
        AudioChunk* ac = get();

        if (terminate)
            break;

        if (is_flusher(ac))
        {
            speakerQueue->flowOn();
        }
        else
        {
            AudioChunk *dAc = new AudioChunk();

            acodec->decode((uint8_t*)ac->data, ac->NumberOfPayloadByte,
                    (uint8_t*)dAc->data, &dAc->NumberOfPayloadByte);

            ALOG_D("[RemoteAudioStream %s] decode %d -> %d\n",
                    peerIpAddress.c_str(),
                    ac->NumberOfPayloadByte,
                    dAc->NumberOfPayloadByte);

            speakerQueue->put(dAc);
        }
//        delete ac;
    }

    /*
     * TODO
     * 1. make queue empty
     * 2. release ChannelQueue
     */
}

void RemoteAudioStream::run(void)
{
    t = thread(&RemoteAudioStream::work, this);
}

void RemoteAudioStream::stop(void)
{
    {
        unique_lock<mutex> uniqueLock(lock);
        terminate = true;
        cond.notify_one();
    }

    ALOG_I("[RemoveAudioStream %s] send stop signal\n");

    t.join();
    ALOG_I("[RemoveAudioStream %s] stopped.\n");

}
