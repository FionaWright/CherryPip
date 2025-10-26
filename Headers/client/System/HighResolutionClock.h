//
// Created by fionaw on 26/10/2025.
//

#ifndef PT_HIGHRESOLUTIONCLOCK_H
#define PT_HIGHRESOLUTIONCLOCK_H

#include <chrono>

using std::chrono::high_resolution_clock;

struct TimeArgs
{
    double ElapsedTime;
    double TotalTime;
};

class HighResolutionClock
{
public:
    HighResolutionClock();

    void Tick();

    void Reset();

    double GetDeltaNanoseconds() const;
    double GetDeltaMicroseconds() const;
    double GetDeltaMilliseconds() const;
    double GetDeltaSeconds() const;

    double GetTotalNanoseconds() const;
    double GetTotalMicroseconds() const;
    double GetTotalMilliSeconds() const;
    double GetTotalSeconds() const;

    TimeArgs GetTimeArgs() const;

private:
    high_resolution_clock::time_point m_t0;

    high_resolution_clock::duration m_deltaTime;
    high_resolution_clock::duration m_totalTime;
};

#endif //PT_HIGHRESOLUTIONCLOCK_H