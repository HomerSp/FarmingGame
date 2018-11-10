#pragma once

#include <array>
#include <chrono>
#include <string>
#include <unordered_map>

namespace engine {
class FrameTimer {
public:
    FrameTimer();

    void reset();
    double operator[](size_t i);

private:
    std::unordered_map<size_t, std::pair<bool, std::chrono::time_point<std::chrono::steady_clock>>> mSaved;
};
}
