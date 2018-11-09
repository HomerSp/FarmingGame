#pragma once

#include <chrono>
#include <string>

namespace engine {
class FrameTimer {
public:
    FrameTimer();

    float diff() const;
    double elapsed() const;
    double last() const;

    void start();
    void end();

private:
    std::chrono::time_point<std::chrono::steady_clock> mStart;
    double mSaved;
    double mLast;
};
}
