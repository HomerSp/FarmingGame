#include <engine/camera.h>
#include <engine/clock.h>
#include <engine/graphics/renderer.h>
#include <engine/logger.h>
#include <engine/random.h>
#include <engine/weather.h>

using namespace engine;

Weather::Data::Data()
    : Data(Weather::Sunny, 0.0f, 0.0f, 0.0f)
{
}

Weather::Data::Data(Weather::Type type, float_t intensity, uint8_t windDirection, float_t windSpeed)
    : type(type)
    , intensity(intensity)
    , windDirection(windDirection)
    , windSpeed(windSpeed)
{
}

Weather::Weather(graphics::Renderer& renderer)
    : mSizeMod(0.0f)
    , mData({})
{
    mData[0] = Data(Sunny, Random::range(100) / 100.0f, (Random::range(200) - 100) / 100.0f, Random::range(100) / 100.0f);
    mData[1] = Data(Rain, Random::range(100) / 100.0f, (Random::range(200) - 100) / 100.0f, Random::range(100) / 100.0f);

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
    return mData[0].type;
}

uint8_t Weather::windDirection() const
{
    return mData[0].windDirection;
}

float_t Weather::windSpeed() const
{
    return mData[0].windSpeed;
}

void Weather::drawWater(graphics::Renderer& renderer, Camera& camera)
{
    mWaterParticles->draw(renderer, camera);
}

void Weather::drawWeather(graphics::Renderer& renderer, Camera& camera)
{
    if (mData[0].type == Rain) {
        if (mSnowParticles->enabled()) {
            mSnowParticles->draw(renderer, camera);
        } else {
            mRainParticles->draw(renderer, camera);
        }
    }
}

void Weather::processAsync(uint64_t frameDiff, Camera& camera, Clock& clock)
{
    if (mData[0].type == Sunny || mData[0].type == Cloudy) {
        mWaterParticles->setEnabled(clock.daylight());
        mWaterParticles->processAsync(frameDiff, camera, *this);
    }

    // Rain or snow
    if (mData[0].type == Rain) {
        bool winter = clock.month() == Clock::Winter;
        mSnowParticles->setEnabled(winter);
        mRainParticles->setEnabled(!winter);
        if (winter) {
            mSnowParticles->processAsync(frameDiff, camera, *this);
        } else {
            mRainParticles->processAsync(frameDiff, camera, *this);
        }
    }
}

void Weather::setSize(const Types::Dimension<uint32_t>& size)
{
    mWaterParticles->setSpawnRect(Types::Rect<float_t>(-16, -16, size.width + 32, size.height + 32));
    mSnowParticles->setSpawnRect(Types::Rect<float_t>(-static_cast<int32_t>(size.width / 2), -static_cast<int32_t>(size.height / 2), size.width * 2, size.height * 2));
    mRainParticles->setSpawnRect(Types::Rect<float_t>(-16, -static_cast<int32_t>(size.height / 2), size.width + 32, size.height * 2));

    mSizeMod = (size.width + size.height) / 1000.0f;
    updateIntensity();
}

void Weather::updateIntensity()
{
    mWaterParticles->setCount(40 * mSizeMod);
    mSnowParticles->setCount(70 * mSizeMod * mData[0].intensity * 10.0f);
    mRainParticles->setCount(20 * mSizeMod * mData[0].intensity * 10.0f);
}