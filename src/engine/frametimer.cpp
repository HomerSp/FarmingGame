#include <engine/frametimer.h>

using namespace engine;

FrameTimer::FrameTimer()
    : mSet(false)
{
}

double FrameTimer::diff()
{
    auto n = std::chrono::steady_clock::now();
    if (!mSet) {
        mLast = n;
        mSet = true;
    }

    auto r = std::chrono::duration_cast<std::chrono::duration<double>>(n - mLast);
    mLast = n;
    return r.count();
}

void FrameTimer::reset()
{
    mSet = false;
}
