#include "MultimediaControl.h"
#include "MultimediaManager.h"
#include <iostream>
#include <shared_mutex>
#include "../MarsLog.h"

using namespace std;

MultimediaControl MARSMultimediaControl;

MultimediaControl::MultimediaControl()
    : onDestroy(false), vhandler(nullptr), ahandler(nullptr)
{
    cout << "[MultimediaControl] created\n";
}

MultimediaControl::~MultimediaControl()
{
    onDestroy = true;
    cout << "[MultimediaControl] destroy\n";
}

void MultimediaControl::addVideoEventHandler(VideoEventHandler* handler)
{
    {
        unique_lock<shared_mutex> wlock(lock);
        vhandler = handler;
    }

    /* inter-coupling, have to remove it!! */
    MARSMultimediaManager.cameraStart();
}

void MultimediaControl::delVideoEventHandler(void)
{
    {
        unique_lock<shared_mutex> wlock(lock);
        vhandler = nullptr;
    }

    /* inter-coupling, have to remove it!! */
    MARSMultimediaManager.cameraStop();
}

void MultimediaControl::addAudioEventHandler(AudioEventHandler* handler)
{
    unique_lock<shared_mutex> wlock(lock);

    ahandler = handler;
}

void MultimediaControl::delAudioEventHandler(void)
{
    unique_lock<shared_mutex> wlock(lock);
    ahandler = nullptr;
}

bool MultimediaControl::sendVideoFrame(uint8_t* data, int size, int id)
{
    if (onDestroy)
        return false;

    shared_lock<shared_mutex> rlock(lock);

    if (!vhandler)
    {
        LOG_I("[MultimediaControl] no video handler from UI\n");
        return false;
    }

    return vhandler->onFrameArrive(data, size, id);
}

bool MultimediaControl::sendVideoFrame(VideoFrame* vf, int id)
{
    if (onDestroy)
        return false;

    return sendVideoFrame(vf->data, vf->bytesUsed, id);
}

bool MultimediaControl::sendVideoFrame(VideoFrame vf, int id)
{
    if (onDestroy)
        return false;

    return sendVideoFrame(vf.data, vf.bytesUsed, id);
}

bool MultimediaControl::sendVadStatus(bool isSpeaking)
{
    if (onDestroy)
        return false;

    shared_lock<shared_mutex> rlock(lock);

    if (!ahandler)
    {
        LOG_I("[MultimediaControl] no audio handler from UI\n");
        return false;
    }

    return ahandler->onNotifyVadStatus(isSpeaking, 0);
}
