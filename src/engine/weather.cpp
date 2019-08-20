#include <engine/random.h>
#include <engine/weather.h>

using namespace engine;

Weather::Weather()
{
    mWeatherIntensity = Random::range(100, 200) / 100.0f;
    mWindDirection = Random::range(0, 360);
    mWindSpeed = Random::range(0, 200) / 100.0f;

    mWaterParticles = std::make_unique<engine::Particles>(30, graphics::Color(1, 1, 0.8f, 0.5f), Types::Dimension<>(2, 2));
    mSnowParticles = std::make_unique<engine::Particles>(30, graphics::Color(1, 1, 1, 0.75f), Types::Dimension<>(2, 2), true);
}

float_t Weather::windDirectionRad() const
{
    return (mWindDirection * Types::PI()) / 180.0f;
}

float_t Weather::windSpeed() const
{
    return mWindSpeed;
}

void Weather::drawWater(graphics::Renderer& renderer, Camera& camera)
{
    mWaterParticles->lock();
    renderer.drawParticles({-camera.x(), -camera.y()}, *mWaterParticles);
    mWaterParticles->unlock();
}

void Weather::drawWeather(graphics::Renderer& renderer, Camera& camera)
{
    mSnowParticles->lock();
    renderer.drawParticles({-camera.x(), -camera.y()}, *mSnowParticles);
    mSnowParticles->unlock();
}

void Weather::processAsync(uint64_t frameDiff, Camera& camera, Clock& clock)
{
    mWaterParticles->setEnabled(clock.daylight());
    mWaterParticles->processAsync(frameDiff, camera, *this);
    mSnowParticles->processAsync(frameDiff, camera, *this);
}

void Weather::setSize(const Types::Dimension<uint32_t>& size)
{
    float_t mod = (size.width + size.height) / 1000.0f;
    mWaterParticles->setCount(10 * mod);
    mSnowParticles->setCount(50 * mod * mWeatherIntensity);
}
