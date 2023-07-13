#pragma once
#include <iostream>
#include <string>
#include <thread>

class PCMRepeater {
    int sampleRate;
    int channel;
    double dropProbbility;
    double jitterMeanMs;
    double jitterStddevMs;

    std::string filename;
    std::thread t;

    void work(void);

public:
    PCMRepeater(std::string n, int sr, int chan);
    PCMRepeater(std::string n, int sr, int chan, double prob, double mean, double stddev);
    ~PCMRepeater();
    void run(void);
};
