#include <iomanip>

#include <engine/clock.h>
#include <engine/context.h>
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
    , mChangedDay(false)
    , mChangedDaylight(false)
{
}

uint64_t Clock::current() const
{
    return mCurrent;
}

float_t Clock::currentHour() const
{
    return (mCurrent % (24 * 60)) + mCurrentMod;
}

uint8_t Clock::year() const
{
    return static_cast<uint8_t>(std::floor(mCurrent / 60.0f / 24.0f / 30.0f / 4.0f));
}

uint8_t Clock::month() const
{
    return static_cast<uint8_t>(std::floor(mCurrent / 60.0f / 24.0f / 30.0f)) % 4;
}

uint8_t Clock::day() const
{
    return static_cast<uint8_t>(std::floor(mCurrent / 60.0f / 24.0f)) % 30;
}

uint8_t Clock::week() const
{
    return static_cast<uint8_t>(std::floor(mCurrent / 60.0f / 24.0f / 7.0f)) % 4;
}

uint8_t Clock::weekDay() const
{
    return static_cast<uint8_t>(std::floor(mCurrent / 60.0f / 24.0f)) % 7;
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

uint8_t Clock::yearDisplay() const
{
    return 1 + year();
}

uint8_t Clock::monthDisplay() const
{
    return 1 + month();
}

uint8_t Clock::dayDisplay() const
{
    return 1 + day();
}

uint8_t Clock::weekDisplay() const
{
    return 1 + week();
}

uint8_t Clock::weekDayDisplay() const
{
    return 1 + weekDay();
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
    str << std::setw(2) << std::setfill('0') << static_cast<int32_t>(hour());
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
    str << std::setw(2) << std::setfill('0') << static_cast<int32_t>(dayDisplay());
    return str.str();
}

std::string Clock::weekDayFormattedShort() const
{
    static std::array<std::string, 8> svalues = {"Mon", "Tue", "Wed", "Thu", "Fri", "Sat", "Sun"};
    return svalues.at(weekDay());
}

bool Clock::processAsync(uint64_t frameDiff, Map* map)
{
    float_t mod = mCurrentMod + (frameDiff / 1000.0f);

    uint64_t current = mCurrent;
    uint64_t val = current + std::floor(mod);

    bool changed = val != current;
    if (changed) {
        current = val;
        mod -= std::floor(mod);

        if (val % 5 == 0) {
            std::lock_guard<std::mutex> lock(mListenerMutex);
            for(auto& i: mChangeListeners) {
                i->check(current);
            }
        }

        map->toggleLights(!daylight());
    }

    uint8_t prevDay = day();
    bool prevDaylight = daylight();

    mCurrentMod = mod;
    mCurrent = current;

    if (changed) {
        if (prevDay != day()) {
            mChangedDay = true;
        }

        if (prevDaylight != daylight()) {
            mChangedDaylight = true;
        }
    }

    return changed;
}

void Clock::processListeners()
{
    Listeners::maybeTrigger(scriptContext(), mListenerMutex, mChangeListeners);

    if (mChangedDay) {
        trigger(DayChanged);
        mChangedDay = false;
    }

    if (mChangedDaylight) {
        trigger(DaylightChanged);
        mChangedDaylight = false;
    }
}

void Clock::fastForward(float_t v)
{
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

void Clock::delay(const std::string& format, asIScriptFunction* func)
{
    std::lock_guard<std::mutex> lock(mListenerMutex);
    mChangeListeners.push_back(std::make_shared<ChangeListener>(func, format, mCurrent + mCurrentMod));
}

void Clock::on(const std::string& type, const std::string& format, asIScriptFunction* func)
{
    if (type == "change") {
        std::lock_guard<std::mutex> lock(mListenerMutex);
        mChangeListeners.push_back(std::make_shared<ChangeListener>(func, format));
    } else {
        Logger::warning("Clock") << "on, unknown trigger" << type;
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
    REGISTER_FUNC_ARGS(engine, Clock, void, delay, const std::string, script::ScriptCallback&&);
    REGISTER_FUNC_ARGS(engine, Clock, void, on, const std::string, const std::string, script::ScriptCallback&&);
}

Clock::ChangeListener::ChangeListener(asIScriptFunction* fun, const std::string& format, uint64_t delayStart)
    : Listener(fun, delayStart > 0)
    , mDelay(delayStart > 0)
    , mTriggered(false)
{
    mTime = Time::fromString(format, delayStart);
}

bool Clock::ChangeListener::check(uint64_t val)
{
    Time current = Time::fromCurrent(val);
    if ((mDelay && current < mTime) || (!mDelay && !mTime.equals(current))) {
        mTriggered = false;
        return false;
    }

    bool t = mTriggered;
    if (!t) {
        setCanTrigger(true);
    }

    mTriggered = true;
    return !t;
}
