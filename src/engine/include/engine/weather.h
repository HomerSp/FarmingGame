#pragma once

#include <mutex>

#include <engine/particles.h>
#include <engine/types.h>

namespace engine {

namespace graphics {
class Renderer;
}

class Camera;
class Clock;

class Weather {
public:
    typedef enum {
        Sunny = 0,
        Cloudy,
        Overcast,
        Rain,
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
    Weather(graphics::Renderer& renderer);

    Weather::Type type() const;
    uint8_t windDirection() const;
    float_t windSpeed() const;

    void drawWater(graphics::Renderer& renderer, Camera& camera);
    void drawWeather(graphics::Renderer& renderer, Camera& camera);

    void processAsync(uint64_t frameDiff, Camera& camera, Clock& clock);

    void setSize(const Types::Dimension<uint32_t>& size);

private:
    void updateIntensity();

    std::mutex mMutex;

    float_t mSizeMod;
    std::array<Weather::Data, 2> mData;

    std::unique_ptr<Particles> mWaterParticles;
    std::unique_ptr<Particles> mSnowParticles;
    std::unique_ptr<Particles> mRainParticles;
};
}
