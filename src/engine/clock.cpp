#include <algorithm>
#include <cstdlib>
#include <iomanip>

#include <engine/clock.h>
#include <engine/logger.h>
#include <engine/map.h>
#include <engine/time.h>

using namespace engine;

Clock::Clock(std::shared_ptr<Context> &ctx)
    : ScriptObject(ctx)
    , mCurrent(0)
    , mCurrentMod(0.0f)
    , mDawn(6)
    , mDusk(20)
{
}

uint64_t Clock::current() const
{
    return mCurrent;
}

uint8_t Clock::year() const
{
    return 1 + static_cast<uint8_t>(std::floor(mCurrent / 60.0f / 24.0f / 30.0f / 4.0f));
}

uint8_t Clock::month() const
{
    return 1 + static_cast<uint8_t>(std::floor(mCurrent / 60.0f / 24.0f / 30.0f)) % 4;
}

uint8_t Clock::day() const
{
    return 1 + static_cast<uint8_t>(std::floor(mCurrent / 60.0f / 24.0f)) % 30;
}

uint8_t Clock::week() const
{
    return 1 + static_cast<uint8_t>(std::floor(mCurrent / 60.0f / 24.0f / 7.0f)) % 4;
}

uint8_t Clock::weekDay() const
{
    return 1 + static_cast<uint8_t>(std::floor(mCurrent / 60.0f / 24.0f)) % 7;
}

uint8_t Clock::hour() const
{
    return static_cast<uint8_t>(std::floor(mCurrent / 60.0f)) % 24;
}

uint8_t Clock::minute() const
{
    return static_cast<uint8_t>(mCurrent % 60);
}

uint8_t Clock::minuteRounded() const
{
    auto m = static_cast<uint8_t>(mCurrent % 60);
    return (std::floor(m / 10) * 10) + ((m % 10 < 5) ? 0 : 5);
}

uint8_t Clock::dawn() const
{
    return mDawn - sunMonthMod();
}

uint8_t Clock::sunrise() const
{
    return dawn() + 2;
}

uint8_t Clock::sunset() const
{
    return dusk() - 2;
}

uint8_t Clock::dusk() const
{
    return mDusk + sunMonthMod();
}

uint8_t Clock::sunMonthMod() const
{
    uint8_t m = month();
    switch (m)  {
    case 2:
        return 2;
    case 4:
        return -2;
    default:
        return 0;
    }
}

bool Clock::daylight() const
{
    auto h = hour();
    return h >= sunrise() && h < sunset();
}

std::string Clock::timeFormatted() const
{
    std::stringstream str;
    str << std::setw(2) << std::setfill('0') << static_cast<int32_t>(static_cast<uint8_t>(std::floor(mCurrent / 60.0f)) % 24);
    if (mCurrent % 2 == 0) {
        str << " ";
    } else {
        str << ":";
    }
    str << std::setw(2) << std::setfill('0') << static_cast<int32_t>(minuteRounded());
    return str.str();
}

std::string Clock::dayFormatted() const
{
    std::stringstream str;
    str << std::setw(2) << std::setfill('0') << static_cast<int32_t>(day());
    return str.str();
}

std::string Clock::weekDayFormattedShort() const
{
    switch (weekDay()) {
    case 2:
        return "Tue";
    case 3:
        return "Wed";
    case 4:
        return "Thu";
    case 5:
        return "Fri";
    case 6:
        return "Sat";
    case 7:
        return "Sun";
    }

    return "Mon";
}

bool Clock::processAsync(uint64_t frameDiff, Map* map)
{
    mCurrentMod += (frameDiff / 1000.0f);

    uint64_t current = mCurrent;
    uint64_t val = current + std::floor(mCurrentMod);

    bool changed = false;
    if (val != current) {
        mCurrent = val;
        mCurrentMod -= std::floor(mCurrentMod);

        if (val % 5 == 0) {
            std::lock_guard<std::mutex> lock(mListenerMutex);
            for(auto& i: mChangeListeners) {
                i->check(mCurrent);
            }
        }

        map->toggleLights(!daylight());

        changed = true;
    }

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

void Clock::fastForward(float_t v) {
    mCurrentMod += v;
}

void Clock::setTime(int32_t h, int32_t m)
{
    if(h > 23 || m > 59) {
        return;
    }

    int32_t v = m + (h * 60);
    int32_t c = minute() + (hour() * 60);
    mCurrentMod += (v - c);
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
    REGISTER_FUNC_ARGS(engine, Clock, void, on, const std::string, const std::string, script::ScriptCallback&&);
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
    std::unordered_map<Time::Type, int8_t> data;
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
        auto v = static_cast<int8_t>(std::floor(val / 60.0f / 24.0f / 28.0f / 4.0f));
        if (mYear != v + 1) {
            mTriggered = false;
            return false;
        }
    }

    if (mMonth != -1) {
        auto v = static_cast<int8_t>(std::floor(val / 60.0f / 24.0f / 28.0f)) % 4;
        if (mMonth != v + 1) {
            mTriggered = false;
            return false;
        }
    }

    if (mDay != -1) {
        auto v = static_cast<int8_t>(std::floor(val / 60.0f / 24.0f)) % 28;
        if (mDay != v + 1) {
            mTriggered = false;
            return false;
        }
    }

    if (mWeek != -1) {
        auto v = static_cast<int8_t>(std::floor(val / 60.0f / 24.0f / 7.0f)) % 4;
        if (mWeek != v + 1) {
            mTriggered = false;
            return false;
        }
    }

    if (mWeekDay != -1) {
        auto v = static_cast<int8_t>(std::floor(val / 60.0f / 24.0f)) % 7;
        if (mWeekDay != v + 1) {
            mTriggered = false;
            return false;
        }
    }

    if (mHour != -1) {
        auto v = static_cast<int8_t>(std::floor(val / 60.0f)) % 24;
        if (mHour != v) {
            mTriggered = false;
            return false;
        }
    }

    if (mMinute != -1) {
        auto v = static_cast<int8_t>(val % 60);
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
