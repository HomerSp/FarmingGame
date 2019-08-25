#pragma once

#include <mutex>

#include <engine/camera.h>
#include <engine/clock.h>
#include <engine/particles.h>
#include <engine/graphics/renderer.h>
#include <engine/types.h>

namespace engine {
class Weather {
public:
    typedef enum {
        Sunny = 0,
        Cloudy,
        Overcast,
        Rain,
    } Type;
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
    std::mutex mMutex;

    Weather::Type mType;
    float_t mIntensity;
    uint8_t mWindDirection;
    float_t mWindSpeed;

    std::unique_ptr<Particles> mWaterParticles;
    std::unique_ptr<Particles> mSnowParticles;
    std::unique_ptr<Particles> mRainParticles;
};
}
