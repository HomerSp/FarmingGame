#pragma once

#include <chrono>
#include <string>

namespace engine {
class FrameTimer {
public:
    FrameTimer();

    double diff();
    void reset();

private:
    bool mSet;
    std::chrono::time_point<std::chrono::steady_clock> mLast;
};
}
