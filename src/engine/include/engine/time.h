#pragma once

#include <cmath>

#include <engine/renderer.h>
#include <engine/types.h>

namespace engine {
class Time {
public:
    Time();

    uint32_t day() const;
    uint32_t hour() const;
    uint32_t minute() const;

    void fastForward(float v) {
        mCurrent += v;
    }

    void draw(Renderer& renderer);
    void process(uint64_t frameDiff);

private:
    double mCurrent;
    uint32_t mDawn;
    uint32_t mSunrise;
    uint32_t mSunset;
    uint32_t mDusk;
};
} 
