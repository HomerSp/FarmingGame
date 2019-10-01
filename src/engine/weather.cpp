#include <engine/camera.h>
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

Weather::Data::Data(Weather::Type type, float_t intensity, float_t windDirection, float_t windSpeed)
    : type(type)
    , intensity(intensity)
    , windDirection(windDirection)
    , windSpeed(windSpeed)
{
    if (type == Weather::Storm) {
        this->intensity += 2.0f;
        this->windSpeed *= 2.0f;
    }
}

Weather::Weather(std::shared_ptr<Context>& ctx, graphics::Renderer& renderer, Clock& clock)
    : ContextObject(ctx)
    , mPercentages({})
    , mSizeMod(0.0f)
    , mData({})
{
    auto docPtr = context().assetManager().data(AssetManager::Config, "weather");
    Json::Value doc = *docPtr;
    if (!doc.isObject() || !doc.isMember("spring") || !doc.isMember("summer") || !doc.isMember("autumn") || !doc.isMember("winter")) {
        Logger::critical("Weather") << "Invalid JSON data for config weather";
        return;
    }

    auto percentFunc = [&](const std::string& name, int s) {
        Json::Value springObj = doc[name];
        for (uint32_t i = 0; i < springObj.size(); i++) {
            mPercentages.at(s).at(i) = springObj[i].asInt();
        }
    };
    
    percentFunc("spring", Clock::Spring);
    percentFunc("summer", Clock::Summer);
    percentFunc("autumn", Clock::Autumn);
    percentFunc("winter", Clock::Winter);

    mData[0] = Data(randomType(clock), Random::range(10, 100) / 100.0f, Random::range(3600) / 10.0f, Random::range(100) / 100.0f);
    mData[1] = Data(randomType(clock), Random::range(10, 100) / 100.0f, Random::range(3600) / 10.0f, Random::range(100) / 100.0f);

    mWaterParticles = std::make_unique<engine::Particles>(renderer, 30, graphics::Color(1, 1, 0.8f, 0.5f), Types::Dimension<>(2, 2));
    mWaterParticles->setMoveSpeed(1.0f);
    mSnowParticles = std::make_unique<engine::Particles>(renderer, 30, graphics::Color(1, 1, 1, 1.0f), Types::Dimension<>(10, 10), false);
    mSnowParticles->setFrameSpeed(500.0f);
    mSnowParticles->setMoveSpeed(100.0f);
    mSnowParticles->setLifeRange(50, 100);
    mSnowParticles->setRound(true);
    mRainParticles = std::make_unique<engine::Particles>(renderer, 30, graphics::Color(1, 1, 1, 1.0f), Types::Dimension<>(3, 60), true);
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

float_t Weather::windDirection() const
{
    return mData[0].windDirection;
}

float_t Weather::windSpeed() const
{
    return mData[0].windSpeed;
}

void Weather::drawWater(graphics::Renderer& renderer, const Types::Point<> dst)
{
    mWaterParticles->draw(renderer, dst);
}

void Weather::drawWeather(graphics::Renderer& renderer, const Types::Point<> dst)
{
    mSnowParticles->draw(renderer, dst);
    mRainParticles->draw(renderer, dst);
}

void Weather::processAsync(uint64_t frameDiff, Camera& camera)
{
    mWaterParticles->processAsync(frameDiff, camera, *this);

    // Rain or snow
    mSnowParticles->processAsync(frameDiff, camera, *this);
    mRainParticles->processAsync(frameDiff, camera, *this);
}

void Weather::setSize(const Types::Dimension<uint32_t>& size)
{
    mWaterParticles->setSpawnRect(Types::Rect<float_t>(-16, -16, size.width + 32, size.height + 32));
    mSnowParticles->setSpawnRect(Types::Rect<float_t>(-static_cast<int32_t>(size.width / 2), -static_cast<int32_t>(size.height / 2), size.width * 2, size.height * 2));
    mRainParticles->setSpawnRect(Types::Rect<float_t>(-static_cast<int32_t>(size.width / 2), -static_cast<int32_t>(size.height / 2), size.width * 2, size.height * 2));

    mSizeMod = (size.width + size.height) / 1000.0f;
    updateIntensity();
}

void Weather::dayChanged(const Clock& clock)
{
    mData[0] = mData[1];
    mData[1] = Data(randomType(clock), Random::range(10, 100) / 100.0f, Random::range(3600) / 10.0f, Random::range(100) / 100.0f);

    updateParticles(clock);
}

void Weather::daylightChanged(const Clock& clock)
{
    if (mData[0].type == Clear) {
        mWaterParticles->setEnabled(clock.daylight());
    }
}

Weather::Type Weather::randomType(const Clock& clock)
{
    uint32_t i = 0, total = 0;
    auto r = Random::range(0, 100);
    for (auto per: mPercentages.at(clock.month())) {
        total += per;
        if (r < total) {
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
    mSnowParticles->setCount(20 * mSizeMod * mData[0].intensity * 10.0f);
    mRainParticles->setCount(20 * mSizeMod * mData[0].intensity * 10.0f);
}

void Weather::updateParticles(const Clock& clock)
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