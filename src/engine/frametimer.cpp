#include <engine/frametimer.h>

using namespace engine;

FrameTimer::FrameTimer()
    : mLast(std::chrono::steady_clock::now())
    , mStart(std::chrono::steady_clock::now())
    , mFrames(0)
    , mFrameCounter(0)
{
}

uint64_t FrameTimer::start()
{
    auto n = std::chrono::steady_clock::now();
    uint64_t d = std::chrono::duration_cast<std::chrono::milliseconds>(n - mLast).count();
    if (d > 0) {
        mLast = n;
    }

    mFrames++;

    auto sd = std::chrono::duration_cast<std::chrono::milliseconds>(n - mStart).count() / 1000.0f;
    if (sd >= 0.25f) {
        mFrameCounter = static_cast<uint16_t>(mFrames / sd);
        mFrames = 0;
        mStart = n;
    }

    return d;
}

uint16_t FrameTimer::framesPerSecond() const
{
    return mFrameCounter;
}
