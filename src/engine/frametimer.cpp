#include <engine/frametimer.h>

using namespace engine;

FrameTimer::FrameTimer()
{
    reset();
}

void FrameTimer::reset()
{
    for (auto& i: mSaved) {
        i.second.first = false;
    }
}

double FrameTimer::operator[](size_t i)
{
    std::pair<bool, std::chrono::time_point<std::chrono::steady_clock>>& el = mSaved[i];
    auto n = std::chrono::steady_clock::now();
    if (!el.first) {
        el.first = true;
        el.second = n;
    }

    auto r = std::chrono::duration_cast<std::chrono::duration<double>>(n - el.second);
    el.second = n;
    return r.count();
}