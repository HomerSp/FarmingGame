#include <engine/logger.h>
#include <engine/random.h>
#include <engine/weather.h>

using namespace engine;

Weather::Weather(graphics::Renderer& renderer)
    : mType(Rain)
    , mIntensity(0.0f)
    , mWindDirection(0.0f)
    , mWindSpeed(0.0f)
{
    mIntensity = Random::range(100) / 100.0f;
    mWindDirection = (Random::range(200) - 100) / 100.0f;
    mWindSpeed = Random::range(100) / 100.0f;

    mWaterParticles = std::make_unique<engine::Particles>(renderer, 30, graphics::Color(1, 1, 0.8f, 0.5f), Types::Dimension<>(2, 2));
    mWaterParticles->setMoveSpeed(1.0f);
    mSnowParticles = std::make_unique<engine::Particles>(renderer, 30, graphics::Color(1, 1, 1, 1.0f), Types::Dimension<>(4, 4), true);
    mSnowParticles->setFrameSpeed(500.0f);
    mSnowParticles->setMoveSpeed(100.0f);
    mSnowParticles->setLifeRange(50, 100);
    mRainParticles = std::make_unique<engine::Particles>(renderer, 30, graphics::Color(1, 1, 1, 1.0f), Types::Dimension<>(2, 40), true);
    mRainParticles->setFrameSpeed(100.0f);
    mRainParticles->setMoveSpeed(100.0f);
    mRainParticles->setLifeRange(50, 100);
}

Weather::Type Weather::type() const
{
    return mType;
}

uint8_t Weather::windDirection() const
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
    if (mType == Rain) {
        mSnowParticles->draw(renderer, camera);
        //mRainParticles->draw(renderer, camera);
    }
}

void Weather::processAsync(uint64_t frameDiff, Camera& camera, Clock& clock)
{
    if (mType == Sunny || mType == Cloudy) {
        mWaterParticles->setEnabled(clock.daylight());
        mWaterParticles->processAsync(frameDiff, camera, *this);
    }

    if (mType == Rain) {
        mSnowParticles->processAsync(frameDiff, camera, *this);
        mRainParticles->processAsync(frameDiff, camera, *this);
    }
}

void Weather::setSize(const Types::Dimension<uint32_t>& size)
{
    float_t mod = (size.width + size.height) / 1000.0f;
    mWaterParticles->setCount(20 * mod);
    mSnowParticles->setCount(70 * mod * mIntensity * 10.0f);
    mSnowParticles->setSpawnRect(Types::Rect<float_t>(-static_cast<int32_t>(size.width / 2), -static_cast<int32_t>(size.height / 2), size.width * 2, size.height * 2));
    mRainParticles->setCount(20 * mod * mIntensity * 10.0f);
    mRainParticles->setSpawnRect(Types::Rect<float_t>(-16, -static_cast<int32_t>(size.height / 2), size.width + 32, size.height * 2));
}
