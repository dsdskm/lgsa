#ifndef __MICAUDIOSTREAM_H__
#define __MICAUDIOSTREAM_H__

#include <list>
#include "AudioChunk.h"
#include "AudioStream.h"
#include "MultimediaStreamSender.h"

class MICAudioStream : public AudioStream {
    AudioStreamSender *sender;

    bool ANC; /* Active Noise Cancle */
    bool VAD; /* Voice Detection */
    bool isSpeaking;
    const int vadCntMax;
    int vadCntCur;
    bool alive = false;

    /*
     * How many audio chunk save before voice activated?
     * default: 30 (0.3 sec)
     */
    int preCnt;
    std::list<AudioChunk> preChunks;

    static constexpr int SAMPLE_RATE = 16000;
    static constexpr int CHANNELS = 1;
    static constexpr int BITS_PER_SAMPLE = 16;
    static constexpr int FRAMES_PER_BUFFER = 160;
    static constexpr int BYTES_PER_BUFFER = (FRAMES_PER_BUFFER * sizeof(short));

    virtual void work(void);
    bool boostPriority(void);

    ACodec* mAc = nullptr;
    bool mCurrentVAD = false;

public:
    MICAudioStream(AudioStreamSender *ass, ACodec* ac);
    ~MICAudioStream();

    virtual void run(void);
    virtual void stop(void);
    bool getVadStatus(void);
    void sendVADStatus(bool isSpeaking);
};

#endif
