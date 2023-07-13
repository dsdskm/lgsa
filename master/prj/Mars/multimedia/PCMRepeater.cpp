#include <iostream>
#include <string>
#include <thread>
#include <fstream>
#include <vector>
#include <chrono>
#include <random>
#include <functional>
#include "PCMRepeater.h"
#include "SpeakerRenderer.h"
#include "MultimediaManager.h"
#include "../MarsLog.h"

using namespace std;

PCMRepeater::PCMRepeater(string n, int sr, int chan)
    : PCMRepeater(n, sr, chan, 0, 0, 0.001)
{
}

PCMRepeater::PCMRepeater(string n, int sr, int chan, double prob, double mean, double stddev)
    : filename(n), sampleRate(sr), channel(chan), dropProbbility(prob), jitterMeanMs(mean), jitterStddevMs(stddev)
{
}

PCMRepeater::~PCMRepeater()
{
    if (t.joinable())
        t.join();
}

void PCMRepeater::work(void)
{
    default_random_engine eng(0);
    uniform_real_distribution<double> dist1(0.0, 1.0);
    normal_distribution<double> dist2(jitterMeanMs, jitterStddevMs);

    auto dropGen = bind(dist1, eng);
    auto jitterGen = bind(dist2, eng);

    ifstream file(filename, ios::binary);

    if (!file.is_open())
    {
        cerr << "Failed to open file: " << filename << endl;
        return;
    }

    int bufferPerSecond = sampleRate / 320;

    AudioChannelQueue* queue = MARSMultimediaManager.getSpeaker()->registerProducer();

    while (true)
    {
        AudioChunk *ac = new AudioChunk();

        streampos startPos = file.tellg();

        file.read(ac->data, 320);
        streamsize bytesRead = file.gcount();  // 실제로 읽은 바이트 수

        if (bytesRead == 0)
        {
            file.clear();
            file.seekg(0, ios::beg);
            continue;
        }

        cout << "[PCMRepeater " << filename << " ] put " << bytesRead << " Bytes\n";

        if (dropGen() >= dropProbbility)
        {
            queue->waitNonFull();
            queue->put(ac);
        }
        else
        {
            cout << "[PCMRepeater " << filename << " ] drop packet\n";
            delete ac;
        }

        streampos nextPos = startPos + bytesRead;
        file.seekg(nextPos);

        double frameDelay = static_cast<double>(bufferPerSecond) / sampleRate;
        chrono::milliseconds waitTime(static_cast<long long>(frameDelay * 300));
        this_thread::sleep_for(waitTime);

        if (jitterMeanMs != 0.0)
        {
            double jitterMs = abs(jitterGen());
            cout << "[PCMRepeater " << filename << " ] jitter " << jitterMs << " ms\n";

            chrono::milliseconds jitterWaitTime(static_cast<long long>(jitterMs));
            this_thread::sleep_for(jitterWaitTime);
        }
    }

    file.close();
}

void PCMRepeater::run(void)
{
    t = thread(&PCMRepeater::work, this);
}
