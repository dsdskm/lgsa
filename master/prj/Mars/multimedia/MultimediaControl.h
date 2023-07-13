#pragma once
#include <cstdint>
#include <shared_mutex>
#include "VideoEventHandler.h"
#include "AudioEventHandler.h"
#include "VideoFrame.h"

class MultimediaControl
{
    bool onDestroy;

    VideoEventHandler* vhandler;
    AudioEventHandler* ahandler;

    std::shared_mutex lock;

public:
    MultimediaControl();
    ~MultimediaControl();

    void addVideoEventHandler(VideoEventHandler* handler);
    void delVideoEventHandler(void);
    void addAudioEventHandler(AudioEventHandler* handler);
    void delAudioEventHandler(void);
    bool sendVideoFrame(uint8_t* data, int size, int id);
    bool sendVideoFrame(VideoFrame *vf, int id);
    bool sendVideoFrame(VideoFrame vf, int id);
    bool sendAudioData(uint8_t* data, int size, int id);
    bool sendVadStatus(bool isSpeaking);
};

extern MultimediaControl MARSMultimediaControl;
