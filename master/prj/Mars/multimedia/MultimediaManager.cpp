#include <iostream>
#include <thread>
#include <vector>
#include <list>
#include "MultimediaManager.h"
#include "MultimediaControl.h"
#include "CameraVideoStream.h"
#include "RemoteVideoStream.h"
#include "BypassVCodec.h"
#include "BypassACodec.h"
#include "MjpegVCodec.h"
#include "H264VCodec.h"
#include "BufferedReceiver.h"
#include "MICAudioStream.h"
#include "SpeakerRenderer.h"
#include "PCMRepeater.h"
#include "OPUSACodec.h"
#include "TimeRing.h"
#include "../MarsLog.h"

using namespace std;

enum DEFAULT_VCODEC {
    DEFAULT_VCODEC_MJPEG,
    DEFAULT_VCODEC_H264,
    DEFAULT_VCODEC_BYPASS
};

static DEFAULT_VCODEC defaultVCodec = DEFAULT_VCODEC_H264;

class CameraAdjustment : public MultimediaStreamAdjustment {
    TimeRing tr;
    TimeRing sr;
    int hysteresis;
    mutex lock;

    double sTotalScore;
    double lTotalScore;

    int sLen;
    int lLen;
    list<double> sWindow;
    list<double> lWindow;

    MultimediaManager *manager;
public:
    CameraAdjustment(MultimediaManager *mm)
        : manager(mm), hysteresis(10),
        sTotalScore(0), lTotalScore(0), sLen(3), lLen(10) {}
    ~CameraAdjustment() {}

    void adjustScore(int score) {
        double len1;
        double len2;
        double sAvgScore;
        double lAvgScore;

        {
            unique_lock<mutex> uniqueLock(lock);

            sTotalScore += score;
            sWindow.push_back(score);

            while (sWindow.size() > sLen)
            {
                double prv = sWindow.front();
                sTotalScore -= prv;
                sWindow.pop_front();
            }

            lTotalScore += score;
            lWindow.push_back(score);

            while (lWindow.size() > lLen)
            {
                double prv = lWindow.front();
                lTotalScore -= prv;
                lWindow.pop_front();
            }

            tr.touch();
            double ms = tr.lastLatencyMs();
            if (ms == NAN || ms <= 500)
                return;

            len1 = sWindow.size();
            len2 = lWindow.size();
            sAvgScore = sTotalScore / len1;
            lAvgScore = lTotalScore / len2;
        }

        LOG_I("[CameraAdjustment] short avg %lf long avg %lf\n", sAvgScore, lAvgScore);

        if (len1 == sLen && len2 == lLen && abs(sAvgScore - lAvgScore) >= hysteresis)
        {
            if (sAvgScore > lAvgScore)
            {
                LOG_I("[CameraAdjustment] Network Score Trend Rising %lf -> %lf, Q Up\n", sAvgScore, lAvgScore);
                manager->cameraQualityUp();
            }
            else
            {
                LOG_I("[CameraAdjustment] Network Score Trend Falling %lf -> %lf, Q Down\n", lAvgScore, sAvgScore);
                manager->cameraQualityDown();
            }
        }
    }
};

MultimediaManager MARSMultimediaManager(MARSMultimediaControl);

static CameraAdjustment MARSCameraAdjustment(&MARSMultimediaManager);

SpeakerRenderer* MultimediaManager::getSpeaker(void)
{
    return speaker;
}

void MultimediaManager::multimediaPipeCreate(string ipaddr, string userId, int uiId)
{
    int netId = -1;

    unique_lock<mutex> uniqueLock(lock);

    if (uiIdMap.find(uiId) != uiIdMap.end() || ipMap.find(ipaddr) != ipMap.end())
    {
        LOG_I("[MultimediaManager] duplicate information\n");
        return;
    }

    if (uiId)
    {
        BufferedReceiver<VideoFrame>* br =
            new BufferedReceiver<VideoFrame>(uiId, 10, 5);

        RemoteVideoStream *rvs;

        switch (defaultVCodec)
        {
            case DEFAULT_VCODEC_MJPEG:
                rvs = new RemoteVideoStream(new MjpegVCodec(), ipaddr, br);
                break;
            case DEFAULT_VCODEC_H264:
                rvs = new RemoteVideoStream(new H264VCodec(), ipaddr, br);
                break;
            case DEFAULT_VCODEC_BYPASS:
                rvs = new RemoteVideoStream(new BypassVCodec(), ipaddr, br);
                break;
        }

        /*
        BufferedReceiver<VideoFrame>* br = nullptr;
        RemoteVideoStream *rvs =
            new RemoteVideoStream(new H264VCodec(), ipaddr, uiId);
        */

        RemoteAudioStream *ras =
            new RemoteAudioStream(new OPUSACodec(), ipaddr);

        remoteStreams[ipaddr] =
            new MultimediaStreamReceiver(
                    ipaddr,
                    streamController,
                    rvs,
                    ras,
                    &MARSCameraAdjustment
                    );

        remoteStreams[ipaddr]->run();

        netId = streamController->AddPeer(ipaddr.c_str(), remoteStreams[ipaddr]);

        LOG_I("[MultimediaManager] multimedia pipeline estabilished ip(%s), userid(%s), uiid(%d), netid(%d)\n",
                ipaddr.c_str(), userId.c_str(), uiId, netId);
    }

    ipMap[ipaddr] = make_tuple(userId, uiId, netId);
    uiIdMap[uiId] = make_tuple(ipaddr, userId, netId);

}

/* stop and release pipeline */
void MultimediaManager::multimediaPipeDestroy(string ipaddr, string userId, int uiId)
{
    int _uiId;
    int _netId;
    string _userId;

    unique_lock<mutex> uniqueLock(lock);

    if (ipMap.find(ipaddr) == ipMap.end() ||
        remoteStreams.find(ipaddr) == remoteStreams.end())
    {
        LOG_E("[MultimediaManager] multimediaPipeDestroy: no such peer %s\n",
              ipaddr.c_str());
        return;
    }

    tie(_userId, _uiId, _netId) = ipMap[ipaddr];

    if (_userId != userId || _uiId != uiId)
    {
        LOG_E("[MultimediaManager] multimediaPipeDestroy: invalid peer info\n");
        LOG_E("  we expect (%s:%d) but (%s:%d) for %s\n",
              _userId.c_str(), _uiId, userId.c_str(), uiId, ipaddr.c_str());
        return;
    }

    /* stop chain runner instances */
    MultimediaStreamReceiver *msr = remoteStreams[ipaddr];
    msr->stop();
    delete msr;
    /* TODO release chain instances */
    /********************************/


    /* stop stream */
    streamController->RemovePeer(_netId);

    /* remove hashs */
    remoteStreams.erase(ipaddr);
    ipMap.erase(ipaddr);
    uiIdMap.erase(_uiId);
}

void MultimediaManager::multimediaPipeDestroyByIp(string ipaddr)
{
    int uiId;
    int netId;
    string userId;

    {
        unique_lock<mutex> uniqueLock(lock);

        if (ipMap.find(ipaddr) == ipMap.end())
        {
            LOG_E("[MultimediaManager] multimediaPipeDestroyByIp: no such peer %s\n",
                    ipaddr.c_str());
            return;
        }

        tie(userId, uiId, netId) = ipMap[ipaddr];
    }

    multimediaPipeDestroy(ipaddr, userId, uiId);
}

void MultimediaManager::multimediaPipeDestroyByUserId(string userId)
{
    int uiId;
    string ipaddr;

    bool found = false;

    {
        unique_lock<mutex> uniqueLock(lock);

        for (auto it: ipMap)
        {
            string _userId = get<0>(it.second);

            if (userId == _userId)
            {
                ipaddr = it.first;
                uiId = get<1>(it.second);
                found = true;
                break;
            }
        }
    }

    if (!found)
    {
        LOG_E("[MultimediaManager] multimediaPipeDestroyByUserId: no such peer %s\n",
                userId.c_str());
        return;
    }

    multimediaPipeDestroy(ipaddr, userId, uiId);
}

/* identifier, {id, ip} */
bool MultimediaManager::conferenceCreated(map<int, pair<string, string>>& mapIdWithIp)
{
    for (auto it: mapIdWithIp)
    {
        int uiId = it.first;
        string userId = it.second.first;
        string ipaddr = it.second.second;

        LOG_I("[MultimediaManager] peer information ip(%s), userid(%s), uiid(%d)\n",
                ipaddr.c_str(), userId.c_str(), uiId);

        multimediaPipeCreate(ipaddr, userId, uiId);
    }

    streamController->Start();
    return true;
}

bool MultimediaManager::conferenceDeleted(map<int, pair<string, string>>& mapIdWithIp)
{
    streamController->Stop();

    for (auto it : mapIdWithIp)
    {
        int uiId = it.first;
        string userId = it.second.first;
        string ipaddr = it.second.second;

        LOG_I("[MultimediaManager] peer information ip(%s), userid(%s), uiid(%d)\n",
            ipaddr.c_str(), userId.c_str(), uiId);

        multimediaPipeDestroy(ipaddr, userId, uiId);
    }

    return true;
}


MultimediaManager::MultimediaManager(MultimediaControl& mc) : controller(mc), localVideoStream(nullptr), localAudioStream(nullptr), speaker(nullptr)
{
    int ret;
    cout << "MultimediaManager start\n";

#if 1
    /* streamcontroller run */
    {
        streamController = StreamController::Create(this);
    }
#endif

#if 1
    /* speaker run */
    {
        speaker = new SpeakerRenderer();
        speaker->run();
    }
#endif



#if 0
    map<int, pair<string, string>> m;
    m[1] = {"kim", "127.0.0.1"};
    m[2] = {"kang", "192.168.3.10"};
    m[3] = {"gang", "192.168.3.2" };

    conferenceCreated(m);

    thread t = thread([&]() {
            map<int, pair<string, string>> m;
            m[1] = {"kim", "127.0.0.1"};

            this_thread::sleep_for(chrono::milliseconds(5000));
            MARSMultimediaManager.multimediaPipeDestroyByIp("127.0.0.1");
            MARSMultimediaManager.multimediaPipeDestroyByUserId("kang");
            MARSMultimediaManager.multimediaPipeDestroyByUserId("gang");

            this_thread::sleep_for(chrono::milliseconds(5000));
            MARSMultimediaManager.conferenceCreated(m);

            this_thread::sleep_for(chrono::milliseconds(5000));
            MARSMultimediaManager.multimediaPipeDestroyByIp("127.0.0.1");

            this_thread::sleep_for(chrono::milliseconds(5000));
            MARSMultimediaManager.conferenceCreated(m);
    });

    t.detach();
#endif

#if 0
    string filename1 = "sample1.pcm";
    //PCMRepeater *test1 = new PCMRepeater(filename1, 16000, 1);
    PCMRepeater *test1 = new PCMRepeater(filename1, 16000, 1, 0, 20, 10);
    test1->run();

    string filename2 = "sample2.pcm";
    //PCMRepeater *test2 = new PCMRepeater(filename2, 16000, 1);
    PCMRepeater *test2 = new PCMRepeater(filename2, 16000, 1, 0, 10, 5);
    test2->run();

    string filename3 = "sample3.pcm";
    PCMRepeater *test3 = new PCMRepeater(filename3, 16000, 1);
    test3->run();
#endif
}

MultimediaManager::~MultimediaManager()
{
    cout << "[MultimediaManager] destroy\n";

    delete streamController;
}

bool MultimediaManager::cameraStart(void)
{
#if 1
    /* MIC capture run */
    {
        localAudioStream = new MICAudioStream(new AudioStreamSender(streamController), new OPUSACodec());
        localAudioStream->run();
    }
#endif
    switch (defaultVCodec)
    {
        case DEFAULT_VCODEC_MJPEG:
            localVideoStream = new CameraVideoStream(new VideoStreamSender(streamController), new MjpegVCodec());
            break;
        case DEFAULT_VCODEC_H264:
            localVideoStream = new CameraVideoStream(new VideoStreamSender(streamController), new H264VCodec());
            break;
        case DEFAULT_VCODEC_BYPASS:
            localVideoStream = new CameraVideoStream(new VideoStreamSender(streamController), new BypassVCodec());
            break;
    }

    localVideoStream->run();
    return true;
}

bool MultimediaManager::cameraStop(void)
{
    if (localAudioStream)
    {
        localAudioStream->stop();
        delete localAudioStream;
        localAudioStream = nullptr;
    }

    if (localVideoStream)
    {
        localVideoStream->stop();
        delete localVideoStream;
        localVideoStream = nullptr;
    }
    return true;
}

void MultimediaManager::cameraQualityUp(void)
{
    if (localVideoStream)
    {
        CameraVideoStream *cvs = dynamic_cast<CameraVideoStream*>(localVideoStream);
        cvs->QualityUp();
    }
}

void MultimediaManager::cameraQualityDown(void)
{
    if (localVideoStream)
    {
        CameraVideoStream *cvs = dynamic_cast<CameraVideoStream*>(localVideoStream);
        cvs->QualityDown();
    }
}
