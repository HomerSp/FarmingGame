#pragma once

#include <array>
#include <atomic>
#include <chrono>
#include <string>
#include <unordered_map>

namespace engine {
class FrameTimer {
public:
    FrameTimer();

    uint64_t start();

    uint16_t framesPerSecond() const;

private:
    std::chrono::time_point<std::chrono::steady_clock> mLast;

    // FPS counter
    std::chrono::time_point<std::chrono::steady_clock> mStart;
    uint32_t mFrames;
    std::atomic<uint16_t> mFrameCounter;
};
}
