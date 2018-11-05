#include <engine/logger.h>
#include <engine/time.h>

using namespace engine;

Time::Time()
    : mCurrent(0.0f)
    , mDawn(6)
    , mSunrise(8)
    , mSunset(18)
    , mDusk(20)
{

} 

uint32_t Time::day() const
{
    return static_cast<uint64_t>(std::floor(mCurrent)) / 60 / 24 % 60;
}

uint32_t Time::hour() const
{
    return static_cast<uint64_t>(std::floor(mCurrent)) / 60 % 24;
}

uint32_t Time::minute() const
{
    return static_cast<uint64_t>(std::floor(mCurrent)) % 60;
}

void Time::draw(Renderer& renderer)
{
    uint32_t h = hour();
    if (h >= mSunrise && h < mSunset) {
        return;
    }

    Types::Color color(0, 0, 0, 0);

    // Night
    if (h < mDawn || h >= mDusk) {
        color.a = 175;
    // Sunrise
    } else if (h >= mDawn && h < mSunrise) {
        float diff = ((static_cast<uint64_t>(std::floor(mCurrent)) % (24 * 60)) - (mDawn * 60)) / static_cast<float>((mSunrise - mDawn) * 60);
        float alpha = 175 - 175 * diff;
        color.r = 255 * diff;
        color.a = std::floor(alpha);
    // Sunset
    } else if (h >= mSunset && h < mDusk) {
        float diff = ((static_cast<uint64_t>(std::floor(mCurrent)) % (24 * 60)) - (mSunset * 60)) / static_cast<float>((mDusk - mSunset) * 60);
        float alpha = 175 * diff;
        color.r = 255 - 255 * diff;
        color.a = std::floor(alpha);
    }

    renderer.fillRect({0, 0, renderer.width(), renderer.height()}, color);
}

void Time::process(uint64_t frameDiff)
{
    mCurrent += 1.0f * (frameDiff / 500.0f);
}