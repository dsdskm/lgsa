#ifndef __SPEAKERRENDERER_H__
#define __SPEAKERRENDERER_H__
#include <deque>
#include <vector>
#include <chrono>
#include <thread>
#include <mutex>
#include <atomic>
#include <condition_variable>
#include "AudioChunk.h"

/*
 * Michael & Scott's Lock-Free Queue based + flow control
 * half-lockfree queue, only producer (may) sleep when call waitNonEmpty()
 */
class AudioChannelQueue {
    struct Node
    {
        AudioChunk* data;
        Node* prev;

        Node(AudioChunk* it) : data(it), prev(nullptr) {}
    };

    std::atomic<Node*> head;
    std::atomic<Node*> tail;

    std::atomic<int> len;

    /*
     * flow:
     * Indicates whether the queue is unplugged,
     * that is, whether a consumer (mixer or renderer) can consume or be consumed by the queue.
     *
     * Transitions from false -> true in the following cases
     *  1. grace and normal flow: If the amount of audio chunks in the queue exceeds a threshold (default 1.5 seconds).
     *  2. synced flow: when an explicit sync (flusher) request is received from the peer.
     *  3. timeout flow: more than 1.5 seconds have passed since the last data arrived from the peer.
     *
     * Transition from true -> false in the following cases
     *  1. if the consumer has consumed all of its data. (len == 1 is empty in this lock-free queue)
     */

    std::atomic<bool> flow;
    std::atomic<bool> wait;
    std::atomic<int> maxLen;
    std::atomic<int> threshold;
    std::mutex lock;
    std::condition_variable cond;

public:
    /* total buffer size 1sec, trigger when 200msec data fill */
    AudioChannelQueue() : AudioChannelQueue(100, 20) {}
    AudioChannelQueue(int mxlen, int th) : head(nullptr), tail(nullptr), len(0), wait(false), maxLen(mxlen), threshold(th) {} 
    ~AudioChannelQueue()
    {
        while (tail != nullptr)
        {
            Node* temp = tail;
            tail = tail.load()->prev;
            delete temp->data;
            delete temp;
        }
    }

    int getThreshold(void)
    {
        return threshold;
    }

    int getLen(void)
    {
        return len;
    }

    bool getFlow(void)
    {
        return flow;
    }

    bool IsEmpty(void)
    {
        return len <= 1;
    }

    void flowOn(void)
    {
        flow = true;
    }

    void waitNonFull()
    {
        std::unique_lock<std::mutex> uniqueLock(lock);
        wait = true;
        while (len == maxLen)
            cond.wait(uniqueLock, [this]() { return len < maxLen; });
        wait = false;
    }

    void put(AudioChunk* it)
    {
        Node* newNode = new Node(it);
        newNode->prev = nullptr;

        /* atomic swap */
        Node* prevHead = head.exchange(newNode);

        if (prevHead != nullptr)
            prevHead->prev = newNode;
        else
            tail = newNode;

        len++;

        if (len >= threshold)
            flow = true;
    }

    bool get(AudioChunk** pIt)
    {
        if (!flow)
        {
            if (len < threshold)
                return false;

            flow = true;
        }

        if (len == 1)
        {
            flow = false;
            return false;
        }

        Node* prevTail = tail.load();

        /* CAS */
        while (prevTail != nullptr && !tail.compare_exchange_weak(prevTail, prevTail->prev))
            prevTail = tail.load();

        if (prevTail != nullptr)
        {
            *pIt = prevTail->data;
            delete prevTail;

            int nextLen = len.fetch_sub(1);

            if (wait)
                cond.notify_all();

            if (nextLen == 1)
                flow = false;

            return true;
        }

        return false;
    }
};

class SpeakerRenderer {
    static constexpr int SAMPLE_RATE = 16000;
    static constexpr int CHANNELS = 1;
    static constexpr int BITS_PER_SAMPLE = 16;
    static constexpr int FRAMES_PER_BUFFER = 160;
    static constexpr int BYTES_PER_BUFFER = (FRAMES_PER_BUFFER * sizeof(short));

    std::vector<AudioChannelQueue*> channelQueue;
    std::thread t;
    std::mutex lock;

    void work(void);
    bool SpeakerRenderer::boostPriority(void);

public:
    void run(void);

    AudioChannelQueue* registerProducer(void)
    {
        std::unique_lock<std::mutex> uniqueLock(lock);
        channelQueue.push_back(new AudioChannelQueue());
        return channelQueue.back();
    }
};

#endif
