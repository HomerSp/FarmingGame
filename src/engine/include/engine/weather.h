#pragma once

#include <mutex>

#include <engine/contextobject.h>
#include <engine/particles.h>
#include <engine/types.h>

namespace engine {

namespace graphics {
class Renderer;
}

class Camera;
class Clock;
class Context;

class Weather : public ContextObject {
public:
    typedef enum {
        Clear = 0,
        Overcast,
        Rain,
        Storm,
        First = Clear,
        Last = Storm,
    } Type;

private:
    struct Data {
        Data();
        Data(Weather::Type type, float_t intensity, uint8_t windDirection, float_t windSpeed);

        Weather::Type type;
        float_t intensity;
        uint8_t windDirection;
        float_t windSpeed;
    };

public:
    Weather(std::shared_ptr<Context>& ctx, graphics::Renderer& renderer, Clock& clock);

    bool overcast() const;
    Weather::Type type() const;
    uint8_t windDirection() const;
    float_t windSpeed() const;

    void drawWater(graphics::Renderer& renderer, Camera& camera);
    void drawWeather(graphics::Renderer& renderer, Camera& camera);

    void dayChanged(Clock& clock);

    void processAsync(uint64_t frameDiff, Camera& camera, Clock& clock);

    void setSize(const Types::Dimension<uint32_t>& size);

private:
    Type randomType(Clock& clock);
    void updateIntensity();
    void updateParticles(Clock& clock);

    std::mutex mMutex;

    float_t mSizeMod;
    std::array<Weather::Data, 2> mData;

    std::unique_ptr<Particles> mWaterParticles;
    std::unique_ptr<Particles> mSnowParticles;
    std::unique_ptr<Particles> mRainParticles;
};
}
