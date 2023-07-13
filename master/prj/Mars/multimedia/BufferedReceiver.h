#ifndef __BUFFEREDRECEIVER_H__
#define __BUFFEREDRECEIVER_H__
#include <deque>
#include <chrono>
#include <thread>
#include <mutex>
#include <atomic>
#include <condition_variable>
#include "VideoFrame.h"

template <typename T>
class BufferedReceiver {
    int id;
    std::atomic<int> maxlength;
    std::atomic<int> threshold;
    std::atomic<int> inputRate;
    std::atomic<int> outputRate;

    std::deque<T> queue;
    std::condition_variable cond;
    std::mutex lock;

    std::thread t;
    void work(void);
    bool terminate;

public:
    BufferedReceiver(int id, int mxlen, int th);
    ~BufferedReceiver();

    void setMaxLength(int mxlen);
    void setThreshold(int th);
    int getMaxLength(void);
    int getThreshold(void);

    bool put(T &data);
    T get(void);
    void run(void);
    void stop(void);
};

template class BufferedReceiver<VideoFrame>;
#endif

