#include <iostream>
#include <chrono>
#include "RemoteVideoStream.h"
#include "BufferedReceiver.h"
#include "MultimediaControl.h"
#include "../MarsLog.h"

using namespace std;

RemoteVideoStream::RemoteVideoStream(VCodec* vc, string ip)
: VideoStream(vc), peerIpAddress(ip), uiId(-1), terminate(false), bufferedReceiver(nullptr)
{
    /* connect with transport manager */
    cout << "[Remote Video Stream " << peerIpAddress << " ] Create\n";
}

RemoteVideoStream::RemoteVideoStream(VCodec* vc, string ip, BufferedReceiver<VideoFrame>* br)
: RemoteVideoStream(vc, ip)
{
    bufferedReceiver = br;
    cout << "[Remote Video Stream " << peerIpAddress << " ] with bufferedRecevier\n";
}

RemoteVideoStream::RemoteVideoStream(VCodec* vc, string ip, int uiid)
: RemoteVideoStream(vc, ip)
{
    uiId = uiid;
}

RemoteVideoStream::~RemoteVideoStream()
{
    cout << "[Remote Video Stream " << peerIpAddress << " ] Destroy\n";
}

void RemoteVideoStream::setReceiver(BufferedReceiver<VideoFrame>* br)
{
    bufferedReceiver = br;
}

bool RemoteVideoStream::put(VideoFrame &vf)
{
    unique_lock<mutex> uniqueLock(lock);
    queue.push_back(vf);
    cond.notify_one();
    return true;
}

VideoFrame RemoteVideoStream::get(void)
{
    unique_lock<mutex> uniqueLock(lock);

    while (!terminate && queue.empty())
    {
        if (!cond.wait_for(uniqueLock, chrono::seconds(1),
                    [this]() { return terminate || !queue.empty(); }))
            VLOG_I("[RemoveVideoStream %s] packet receive wait timeout\n", peerIpAddress.c_str());
    }

    if (terminate)
        return VideoFrame();

    VideoFrame vf = queue.front();
    queue.pop_front();

    return vf;
}

void RemoteVideoStream::work(void)
{

    while (!terminate)
    {
        VideoFrame ivf = get();

        if (terminate)
            break;

        size_t osize = 640*480*3;
        uint8_t* odata = new uint8_t[osize];
        VideoFrame ovf = VideoFrame(640, 480, 3, odata, osize);

        /* decode */
        bool ret = vcodec->decode(ivf.data, ivf.bytesUsed, ovf.data, &ovf.bytesUsed);

        delete [] ivf.data;

        if (!ret)
        {
            delete[] ovf.data;
            continue;
        }

        VLOG_D("[RemoveVideoStream %s] decode %d bytes\n", peerIpAddress.c_str(), ovf.bytesUsed);

#if 0
        /* send media controller */
        MARSMultimediaControl.sendVideoFrame(ovf.data, ovf.bytesUsed, uiId);

        delete [] ovf.data;

#else
        /* send to mediabuffer */
        if (bufferedReceiver)
        {
            bufferedReceiver->put(ovf);
            VLOG_D("[RemoveVideoStream %s] put data bufferedReceiver\n", peerIpAddress.c_str());
        }
        else
        {
            VLOG_I("[RemoveVideoStream %s] no bufferedReceiver\n", peerIpAddress.c_str());
            delete[] ovf.data;
        }
#endif
    }

    /*
     * TODO
     * 1. make queue empty
     */
}

void RemoteVideoStream::run(void)
{
    t = thread(&RemoteVideoStream::work, this);

    if (bufferedReceiver)
        bufferedReceiver->run();
}

void RemoteVideoStream::stop(void)
{
    {
        unique_lock<mutex> uniqueLock(lock);
        terminate = true;
        cond.notify_one();
    }

    VLOG_I("[RemoveVideoStream %s] send stop signal\n");

    if (bufferedReceiver)
    {
        bufferedReceiver->stop();
        VLOG_I("[RemoveVideoStream %s] send stop signal to BufferdReceiver\n");
    }

    t.join();
    VLOG_I("[RemoveVideoStream %s] stopped.\n");
}
