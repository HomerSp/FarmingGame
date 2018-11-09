#include <engine/frametimer.h>

using namespace engine;

FrameTimer::FrameTimer()
    : mStart(std::chrono::steady_clock::now())
{
    mSaved = mLast = elapsed();
}

float FrameTimer::diff() const
{
    return elapsed() - mLast;
}

double FrameTimer::elapsed() const
{
    return std::chrono::duration_cast<std::chrono::duration<double>>(std::chrono::steady_clock::now() - mStart).count();
}

double FrameTimer::last() const
{
    return mLast;
}

void FrameTimer::start()
{
    mSaved = elapsed();
}

void FrameTimer::end()
{
    mLast = mSaved;
}