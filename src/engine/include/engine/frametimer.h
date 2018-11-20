#pragma once

#include <array>
#include <chrono>
#include <string>
#include <unordered_map>

namespace engine {
class FrameTimer {
public:
    FrameTimer();

    uint64_t diff();

private:
    std::chrono::time_point<std::chrono::steady_clock> mLast;
};
}
