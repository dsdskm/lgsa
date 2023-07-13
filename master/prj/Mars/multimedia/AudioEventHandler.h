#ifndef __AUDIOEVENTHANDLER_H__
#define __AUDIOEVENTHANDLER_H__

class AudioEventHandler
{
public:
    virtual bool onNotifyVadStatus(bool isSpeaking, int peerid) = 0;
};

#endif
