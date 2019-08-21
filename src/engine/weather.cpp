#include <engine/random.h>
#include <engine/weather.h>

using namespace engine;

Weather::Weather(graphics::Renderer& renderer)
{
    mWeatherIntensity = Random::range(100, 800) / 100.0f;
    mWindDirection = Random::range(0, 360);
    mWindSpeed = Random::range(0, 200) / 100.0f;

    mWaterParticles = std::make_unique<engine::Particles>(renderer, 30, graphics::Color(1, 1, 0.8f, 0.5f), Types::Dimension<>(2, 2));
    mWaterParticles->setMoveSpeed(1.0f);
    mSnowParticles = std::make_unique<engine::Particles>(renderer, 30, graphics::Color(1, 1, 1, 1.0f), Types::Dimension<>(4, 4), true);
    mSnowParticles->setFrameSpeed(1000.0f);
    mSnowParticles->setMoveSpeed(100.0f);
    mSnowParticles->setLifeRange(75, 100);
}

uint16_t Weather::windDirection() const
{
    return mWindDirection;
}

float_t Weather::windSpeed() const
{
    return mWindSpeed;
}

void Weather::drawWater(graphics::Renderer& renderer, Camera& camera)
{
    mWaterParticles->draw(renderer, camera);
}

void Weather::drawWeather(graphics::Renderer& renderer, Camera& camera)
{
    mSnowParticles->draw(renderer, camera);
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
    mWaterParticles->setCount(20 * mod);
    mSnowParticles->setCount(70 * mod * mWeatherIntensity);
}
