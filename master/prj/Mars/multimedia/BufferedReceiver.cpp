#include <iostream>
#include <chrono>
#include "BufferedReceiver.h"
#include "MultimediaControl.h"
#include "TimeRing.h"
#include "../MarsLog.h"

using namespace std;

template <typename T>
BufferedReceiver<T>::BufferedReceiver(int id, int mxlen, int th)
: id(id), maxlength(mxlen), threshold(th), terminate(false), inputRate(0), outputRate(0) {}

template <typename T>
BufferedReceiver<T>::~BufferedReceiver() {}

template <typename T>
void BufferedReceiver<T>::setMaxLength(int mxlen)
{
    maxlength = mxlen;
}

template <typename T>
void BufferedReceiver<T>::setThreshold(int th)
{
    threshold = th;
}

template <typename T>
int BufferedReceiver<T>::getMaxLength()
{
    return maxlength;
}

template <typename T>
int BufferedReceiver<T>::getThreshold()
{
    return threshold;
}

template <typename T>
bool BufferedReceiver<T>::put(T &data)
{
    unique_lock<mutex> uniqueLock(lock);
    if (queue.size() >= maxlength)
        queue.pop_front();

    queue.push_back(data);
    if (queue.size() >= getThreshold())
        cond.notify_one();

    inputRate++;
    return true;
}

template <typename T>
T BufferedReceiver<T>::get(void)
{
    unique_lock<mutex> uniqueLock(lock);

    if (!terminate && queue.empty())
    {
        while (!terminate && (queue.size() < getThreshold()))
            cond.wait(uniqueLock, [this]() { return terminate || (queue.size() >= getThreshold()); });
    }

    if (terminate)
        return T();

    T data = queue.front();
    queue.pop_front();

    outputRate++;
    return data;
}

template <typename T>
void BufferedReceiver<T>::work(void)
{
    TimeRing tr;

    while (!terminate)
    {
        T frame = get();
        VLOG_D("[BufferedReciver %d] get data\n", id);

        if (terminate)
            break;

        double avgMs = tr.touch();
        if (avgMs != NAN)
        {
            double intervalMs = tr.lastLatencyMs();
            if (avgMs > intervalMs)
            {
                chrono::milliseconds waitTime(static_cast<long long>(avgMs - intervalMs));
                this_thread::sleep_for(waitTime);
            }
        }

        /* send media controller */
        MARSMultimediaControl.sendVideoFrame(frame, id);

        delete[] frame.data;
    }

    /*
     * TODO
     * 1. make queue empty
     */
}

template <typename T>
void BufferedReceiver<T>::run(void)
{
    VLOG_I("[BufferedReciver %d] run\n", id);
    t = thread(&BufferedReceiver<T>::work, this);
}

template <typename T>
void BufferedReceiver<T>::stop(void)
{
    {
        unique_lock<mutex> uniqueLock(lock);
        terminate = true;
        cond.notify_one();
    }

    VLOG_I("[BufferedReciver %d] send stop signal\n", id);

    t.join();
    VLOG_I("[BufferedReciver %d] stopped\n", id);
}
