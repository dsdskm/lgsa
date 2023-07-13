#ifndef __MULTIMEDIAMANAGER_H__
#define __MULTIMEDIAMANAGER_H__

#include <vector>
#include <map>
#include <mutex>
#include "VideoStream.h"
#include "AudioStream.h"
#include "MultimediaControl.h"
#include "SpeakerRenderer.h"
#include "MultimediaStreamReceiver.h"
#include "MultimediaStreamSender.h"
#include "../net/StreamController.h"

class MultimediaManager: public net::StreamSenderClient {
    net::StreamController *streamController;

    SpeakerRenderer *speaker;

    MultimediaControl &controller;
    VideoStream* localVideoStream;
    AudioStream* localAudioStream;

    mutex lock;

    /* ip -> MultimediaStreamReceiver instance*/
    map<string, MultimediaStreamReceiver*> remoteStreams;

    /* ip -> <userId, uiid, netid> */
    map<string, tuple<string, int, int>> ipMap;

    /* uiid -> <ipaddr, userId, netid> */
    map<int, tuple<string, string, int>> uiIdMap;

    void multimediaPipeCreate(string ip, string userid, int uiId);
    void multimediaPipeDestroy(string ip, string userid, int uiId);
    void multimediaPipeDestroyByIp(string ip);
    void multimediaPipeDestroyByUserId(string userId);

public:
    MultimediaManager(MultimediaControl& mc);
    ~MultimediaManager();

    SpeakerRenderer* getSpeaker(void);

    virtual void OnComplete(StreamType type, int stream_id, size_t sent_size) {}
    virtual void OnJam() {}

    bool conferenceCreated(map<int, pair<string, string>>& mapIdWithIp);
    bool conferenceDeleted(map<int, pair<string, string>>& mapIdWithIp);

    bool cameraStart(void);
    bool cameraStop(void);
    void cameraQualityUp(void);
    void cameraQualityDown(void);
};

extern MultimediaManager MARSMultimediaManager;

#endif
