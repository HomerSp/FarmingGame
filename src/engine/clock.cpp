#include <cstdlib>

#include <engine/clock.h>
#include <engine/logger.h>
#include <engine/map.h>
#include <engine/time.h>

using namespace engine;

Clock::Clock()
    : mCurrent(0.0f)
    , mDawn(6)
    , mSunrise(8)
    , mSunset(18)
    , mDusk(20)
{
}

double Clock::current() const
{
    return mCurrent;
}

uint32_t Clock::year() const
{
    return 1 + static_cast<uint32_t>(std::floor(mCurrent / 60 / 24 / 30 / 4));
}

uint32_t Clock::month() const
{
    return 1 + static_cast<uint32_t>(std::floor(mCurrent / 60 / 24 / 30)) % 4;
}

uint32_t Clock::day() const
{
    return 1 + static_cast<uint32_t>(std::floor(mCurrent / 60 / 24)) % 30;
}

uint32_t Clock::week() const
{
    return 1 + static_cast<uint32_t>(std::floor(mCurrent / 60 / 24 / 7)) % 4;
}

uint32_t Clock::weekDay() const
{
    return 1 + static_cast<uint32_t>(std::floor(mCurrent / 60 / 24)) % 7;
}

uint32_t Clock::hour() const
{
    return static_cast<uint32_t>(std::floor(mCurrent / 60)) % 24;
}

uint32_t Clock::minute() const
{
    return static_cast<uint32_t>(std::floor(mCurrent)) % 60;
}

uint32_t Clock::dawn() const
{
    return mDawn;
}

uint32_t Clock::sunrise() const
{
    return mSunrise;
}

uint32_t Clock::sunset() const
{
    return mSunset;
}

uint32_t Clock::dusk() const
{
    return mDusk;
}

bool Clock::daylight() const
{
    uint32_t h = static_cast<uint32_t>(std::floor(mCurrent / 60)) % 24;
    return h >= mSunrise && h < mSunset;
}

bool Clock::processAsync(uint64_t frameDiff, Map* map)
{
    uint64_t val = std::floor(mCurrent);
    mCurrent.store(mCurrent + (frameDiff / 1000.0f));

    bool changed = false;
    if (std::floor(mCurrent) != val) {
        std::lock_guard<std::mutex> lock(mListenerMutex);
        for(auto& i: mChangeListeners) {
            i->check(mCurrent);
        }

        changed = true;
    }

    map->toggleLights(!daylight());

    return changed;
}

void Clock::processListeners()
{
    std::vector<ChangeListener *> listeners;
    {
        std::lock_guard<std::mutex> lock(mListenerMutex);
        for (auto& i: mChangeListeners) {
            listeners.emplace_back(i.get());
        }
    }

    auto it = listeners.begin();
    while (it != listeners.end()) {
        (*it)->maybeTrigger(scriptContext());
        it++;
    }
}

void Clock::setTime(int h, int m)
{
    if(h > 23 || m > 59) {
        return;
    }

    int v = m + (h * 60);
    int c = minute() + (hour() * 60);
    mCurrent.store(mCurrent + (v - c));
}

void Clock::on(const std::string& type, const std::string& format, asIScriptFunction* func)
{
    if (type == "change") {
        std::lock_guard<std::mutex> lock(mListenerMutex);
        mChangeListeners.push_back(std::make_shared<ChangeListener>(func, format));
    } else {
        Logger::warning() << "Clock on unknown trigger" << type;
    }
}

std::string Clock::className()
{
    return "Clock";
}

void Clock::registerClass(asIScriptEngine* engine)
{
    registerReference<Clock>(engine);
    REGISTER_FUNC(engine, Clock, uint, year);
    REGISTER_FUNC(engine, Clock, uint, month);
    REGISTER_FUNC(engine, Clock, uint, day);
    REGISTER_FUNC(engine, Clock, uint, week);
    REGISTER_FUNC(engine, Clock, uint, weekDay);
    REGISTER_FUNC(engine, Clock, uint, hour);
    REGISTER_FUNC(engine, Clock, uint, minute);
    REGISTER_FUNC_ARGS(engine, Clock, void, on, const std::string, const std::string, ScriptCallback&&);
}

Clock::ChangeListener::ChangeListener(asIScriptFunction* fun, const std::string& format)
    : Listener(fun)
    , mTriggered(false)
    , mYear(-1)
    , mMonth(-1)
    , mDay(-1)
    , mWeek(-1)
    , mWeekDay(-1)
    , mHour(-1)
    , mMinute(-1)
{
    std::unordered_map<Time::Type, int> data;
    Time::parseString(format, data);

    mYear = (data.find(Time::Year) != data.end()) ? data.find(Time::Year)->second : -1;
    mMonth = (data.find(Time::Month) != data.end()) ? data.find(Time::Month)->second : -1;
    mDay = (data.find(Time::Day) != data.end()) ? data.find(Time::Day)->second : -1;
    mWeek = (data.find(Time::Week) != data.end()) ? data.find(Time::Week)->second : -1;
    mWeekDay = (data.find(Time::WeekDay) != data.end()) ? data.find(Time::WeekDay)->second : -1;
    mHour = (data.find(Time::Hour) != data.end()) ? data.find(Time::Hour)->second : -1;
    mMinute = (data.find(Time::Minute) != data.end()) ? data.find(Time::Minute)->second : -1;
}

bool Clock::ChangeListener::check(uint64_t val)
{
    if (mYear != -1) {
        auto v = static_cast<int>(std::floor(val / 60 / 24 / 28 / 4));
        if (mYear != v + 1) {
            mTriggered = false;
            return false;
        }
    }

    if (mMonth != -1) {
        auto v = static_cast<int>(std::floor(val / 60 / 24 / 28)) % 4;
        if (mMonth != v + 1) {
            mTriggered = false;
            return false;
        }
    }

    if (mDay != -1) {
        auto v = static_cast<int>(std::floor(val / 60 / 24)) % 28;
        if (mDay != v + 1) {
            mTriggered = false;
            return false;
        }
    }

    if (mWeek != -1) {
        auto v = static_cast<int>(std::floor(val / 60 / 24 / 7)) % 4;
        if (mWeek != v + 1) {
            mTriggered = false;
            return false;
        }
    }

    if (mWeekDay != -1) {
        auto v = static_cast<int>(std::floor(val / 60 / 24)) % 7;
        if (mWeekDay != v + 1) {
            mTriggered = false;
            return false;
        }
    }

    if (mHour != -1) {
        auto v = static_cast<int>(std::floor(val / 60)) % 24;
        if (mHour != v) {
            mTriggered = false;
            return false;
        }
    }

    if (mMinute != -1) {
        auto v = static_cast<int>(std::floor(val)) % 60;
        if (mMinute != v) {
            mTriggered = false;
            return false;
        }
    }

    bool t = mTriggered;
    if (!t) {
        setCanTrigger(true);
    }

    mTriggered = true;
    return !t;
}
