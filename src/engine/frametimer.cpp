#include <engine/frametimer.h>

using namespace engine;

FrameTimer::FrameTimer()
{
    mLast = std::chrono::steady_clock::now();
}

uint64_t FrameTimer::diff()
{
    auto n = std::chrono::steady_clock::now();
    uint64_t d = std::chrono::duration_cast<std::chrono::milliseconds>(n - mLast).count();
    if (d > 0) {
        mLast = n;
    }

    return d;
}
