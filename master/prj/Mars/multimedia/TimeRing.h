#ifndef __TIMERING_H__
#define __TIMERING_H__

#include <list>
#include <chrono>

class TimeRing {
    int len;
    bool first;
    double avgMs;
    double totalMs;
    double resetMs;
    std::list<double> window;
    std::chrono::system_clock::time_point lastTp;
    public:
    TimeRing() : len(100), avgMs(0), totalMs(0), first(false), resetMs(1000) {}

    double touch(void) {
        auto curTp = std::chrono::system_clock::now();

        if (!first)
        {
            lastTp = curTp;
            first = true;
            return NAN;
        }

        std::chrono::duration<double> p;
        p = curTp - lastTp;

        double ms = std::chrono::duration_cast<std::chrono::milliseconds>(p).count();

        if (ms >= resetMs)
        {
            window.clear();
            avgMs = 0;
            totalMs = 0;
            lastTp = curTp;
            return NAN;
        }

        totalMs += ms;
        window.push_back(ms);

        while (window.size() > len)
        {
            ms = window.front();
            totalMs -= ms;
            window.pop_front();
        }

        lastTp = curTp;

        if (window.size() < len / 10)
            return NAN;

        avgMs = totalMs / window.size();

        return avgMs;
    }

    double lastLatencyMs(void)
    {
        if (!window.empty())
            return window.back();
        return NAN;
    }
};
#endif

