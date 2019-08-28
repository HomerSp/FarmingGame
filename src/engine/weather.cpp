#include <engine/camera.h>
#include <engine/clock.h>
#include <engine/context.h>
#include <engine/graphics/renderer.h>
#include <engine/logger.h>
#include <engine/random.h>
#include <engine/weather.h>

using namespace engine;

Weather::Data::Data()
    : Data(Weather::Clear, 0.0f, 0.0f, 0.0f)
{
}

Weather::Data::Data(Weather::Type type, float_t intensity, uint8_t windDirection, float_t windSpeed)
    : type(type)
    , intensity(intensity)
    , windDirection(windDirection)
    , windSpeed(windSpeed)
{
    if (type == Weather::Storm) {
        this->intensity += 2.0f;
        this->windSpeed += 2.0f;
    }
}

Weather::Weather(std::shared_ptr<Context>& ctx, graphics::Renderer& renderer, Clock& clock)
    : ContextObject(ctx)
    , mSizeMod(0.0f)
    , mData({})
{
    mData[0] = Data(Storm, Random::range(100) / 100.0f, (Random::range(200) - 100) / 100.0f, Random::range(100) / 100.0f);
    mData[1] = Data(Clear, Random::range(100) / 100.0f, (Random::range(200) - 100) / 100.0f, Random::range(100) / 100.0f);

    mWaterParticles = std::make_unique<engine::Particles>(renderer, 30, graphics::Color(1, 1, 0.8f, 0.5f), Types::Dimension<>(2, 2));
    mWaterParticles->setMoveSpeed(1.0f);
    mSnowParticles = std::make_unique<engine::Particles>(renderer, 30, graphics::Color(1, 1, 1, 1.0f), Types::Dimension<>(4, 4), false);
    mSnowParticles->setFrameSpeed(500.0f);
    mSnowParticles->setMoveSpeed(100.0f);
    mSnowParticles->setLifeRange(50, 100);
    mRainParticles = std::make_unique<engine::Particles>(renderer, 30, graphics::Color(1, 1, 1, 1.0f), Types::Dimension<>(2, 40), true);
    mRainParticles->setFrameSpeed(100.0f);
    mRainParticles->setMoveSpeed(100.0f);
    mRainParticles->setLifeRange(50, 100);

    updateParticles(clock);
}

bool Weather::overcast() const
{
    auto t = mData[0].type;
    return t == Overcast || t == Rain || t == Storm;
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
    mSnowParticles->draw(renderer, camera);
    mRainParticles->draw(renderer, camera);
}

void Weather::dayChanged(Clock& clock)
{
    mData[0] = mData[1];

    auto type = randomType(clock);
    mData[1] = Data(type, Random::range(100) / 100.0f, (Random::range(200) - 100) / 100.0f, Random::range(100) / 100.0f);

    updateParticles(clock);
}

void Weather::processAsync(uint64_t frameDiff, Camera& camera, Clock& clock)
{
    if (mData[0].type == Clear) {
        mWaterParticles->setEnabled(clock.daylight());
    }

    mWaterParticles->processAsync(frameDiff, camera, *this);

    // Rain or snow
    mSnowParticles->processAsync(frameDiff, camera, *this);
    mRainParticles->processAsync(frameDiff, camera, *this);
}

void Weather::setSize(const Types::Dimension<uint32_t>& size)
{
    mWaterParticles->setSpawnRect(Types::Rect<float_t>(-16, -16, size.width + 32, size.height + 32));
    mSnowParticles->setSpawnRect(Types::Rect<float_t>(-static_cast<int32_t>(size.width / 2), -static_cast<int32_t>(size.height / 2), size.width * 2, size.height * 2));
    mRainParticles->setSpawnRect(Types::Rect<float_t>(-16, -static_cast<int32_t>(size.height / 2), size.width + 32, size.height * 2));

    mSizeMod = (size.width + size.height) / 1000.0f;
    updateIntensity();
}

Weather::Type Weather::randomType(Clock& clock)
{
    std::array<int8_t, Type::Last + 1> percentages = {40, 80, 97, 100};

    uint32_t i = 0;
    auto r = Random::range(0, 100);
    for (auto per: percentages) {
        if (r < per) {
            return static_cast<Weather::Type>(i);
            break;
        }

        i++;
    }

    return Type::Storm;
}

void Weather::updateIntensity()
{
    mWaterParticles->setCount(40 * mSizeMod);
    mSnowParticles->setCount(70 * mSizeMod * mData[0].intensity * 10.0f);
    mRainParticles->setCount(20 * mSizeMod * mData[0].intensity * 10.0f);
}

void Weather::updateParticles(Clock& clock)
{
    mWaterParticles->setEnabled(false);

    // Rain or snow
    if (mData[0].type == Rain || mData[0].type == Storm) {
        bool winter = clock.month() == Clock::Winter;
        mSnowParticles->setEnabled(winter);
        mRainParticles->setEnabled(!winter);
    } else {
        mSnowParticles->setEnabled(false);
        mRainParticles->setEnabled(false);
    }
}