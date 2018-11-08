#include <cstdlib>

#include <engine/logger.h>
#include <engine/clock.h>

using namespace engine;

Clock::Clock()
    : mCurrent(0.0f)
    , mDawn(6)
    , mSunrise(8)
    , mSunset(18)
    , mDusk(20)
{
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

void Clock::draw(Renderer& renderer)
{
    uint32_t h = hour();
    if (h >= mSunrise && h < mSunset) {
        return;
    }

    Types::Color color(0, 0, 0, 0);

    // Night
    if (h < mDawn || h >= mDusk) {
        color.a = 175;
    // Sunrise
    } else if (h >= mDawn && h < mSunrise) {
        float diff = ((static_cast<uint64_t>(std::floor(mCurrent)) % (24 * 60)) - (mDawn * 60)) / static_cast<float>((mSunrise - mDawn) * 60);
        float alpha = 175 - 175 * diff;
        color.r = 255 * diff;
        color.a = std::floor(alpha);
    // Sunset
    } else if (h >= mSunset && h < mDusk) {
        float diff = ((static_cast<uint64_t>(std::floor(mCurrent)) % (24 * 60)) - (mSunset * 60)) / static_cast<float>((mDusk - mSunset) * 60);
        float alpha = 175 * diff;
        color.r = 255 - 255 * diff;
        color.a = std::floor(alpha);
    }

    renderer.fillRect({0, 0, renderer.width(), renderer.height()}, color);
}

void Clock::process(uint64_t frameDiff)
{
    uint64_t val = std::floor(mCurrent);
    mCurrent += 1.0f * (frameDiff / 500.0f);

    if (std::floor(mCurrent) != val) {
        auto it = mChangeListeners.begin();
        while (it != mChangeListeners.end()) {
            if ((*it)->check(scriptContext(), mCurrent) && (*it)->once()) {
                it = mChangeListeners.erase(it);
            } else {
                it++;
            }
        }
    }
}

void Clock::setTime(int h, int m)
{
    if(h > 23 || m > 59) {
        return;
    }

    int v = m + (h * 60);
    int c = minute() + (hour() * 60);
    mCurrent += (v - c);
}

void Clock::on(const std::string& type, const std::string& format, asIScriptFunction* func)
{
    if (type == "change") {
        mChangeListeners.push_back(std::make_shared<ChangeListener>(func, format));
    } else {
        Logger::warning() << "Clock on unknown trigger" << type;
    }
}

std::string Clock::className()
{
    return "Clock";
}

void Clock::registerClass()
{
    registerMethod(SCRIPT_FUNC(Clock, uint, year));
    registerMethod(SCRIPT_FUNC(Clock, uint, month));
    registerMethod(SCRIPT_FUNC(Clock, uint, day));
    registerMethod(SCRIPT_FUNC(Clock, uint, week));
    registerMethod(SCRIPT_FUNC(Clock, uint, weekDay));
    registerMethod(SCRIPT_FUNC(Clock, uint, hour));
    registerMethod(SCRIPT_FUNC(Clock, uint, minute));
    registerMethod(SCRIPT_FUNC_ARGS(Clock, void, on, const std::string, const std::string, ScriptCallback&&));
}

Clock::ChangeListener::ChangeListener(asIScriptFunction* fun, const std::string& format, bool once)
    : Listener(fun)
    , mOnce(once)
    , mTriggered(false)
    , mYear(-1)
    , mMonth(-1)
    , mDay(-1)
    , mWeek(-1)
    , mWeekDay(-1)
    , mHour(-1)
    , mMinute(-1)
{
    size_t start = 0;
    if(format.find(',') != std::string::npos) {
        size_t end = 0;
        while((end = format.find(',', end)) != std::string::npos)
        {
            parseBlock(format.substr(start, end));
            end++;
            start = end;
        }
    }

    if (start != std::string::npos) {
        parseBlock(format.substr(start));
    }
}

bool Clock::ChangeListener::check(asIScriptContext& ctx, uint64_t val)
{
    if (mYear != -1) {
        int v = static_cast<int>(std::floor(val / 60 / 24 / 28 / 4));
        if (mYear != v + 1) {
            mTriggered = false;
            return false;
        }
    }

    if (mMonth != -1) {
        int v = static_cast<int>(std::floor(val / 60 / 24 / 28)) % 4;
        if (mMonth != v + 1) {
            mTriggered = false;
            return false;
        }
    }

    if (mDay != -1) {
        int v = static_cast<int>(std::floor(val / 60 / 24)) % 28;
        if (mDay != v + 1) {
            mTriggered = false;
            return false;
        }
    }

    if (mWeek != -1) {
        int v = static_cast<int>(std::floor(val / 60 / 24 / 7)) % 4;
        if (mWeek != v + 1) {
            mTriggered = false;
            return false;
        }
    }

    if (mWeekDay != -1) {
        int v = static_cast<int>(std::floor(val / 60 / 24)) % 7;
        if (mWeekDay != v + 1) {
            mTriggered = false;
            return false;
        }
    }

    if (mHour != -1) {
        int v = static_cast<int>(std::floor(val / 60)) % 24;
        if (mHour != v) {
            mTriggered = false;
            return false;
        }
    }

    if (mMinute != -1) {
        int v = static_cast<int>(std::floor(val)) % 60;
        if (mMinute != v) {
            mTriggered = false;
            return false;
        }
    }

    bool t = mTriggered;
    if (!t) {
        call(ctx);
    }

    mTriggered = true;
    return !t;
}

void Clock::ChangeListener::parseBlock(const std::string& block)
{
    char* end;
    int v = strtol(block.data(), &end, 10);
    if (end[0] != '\0') {
        std::string type(end);
        if(type == "y") {
            mYear = v;
        } else if(type == "mon") {
            mMonth = v;
        } else if(type == "d") {
            mDay = v;
        } else if(type == "w") {
            mWeek = v;
        } else if(type == "wd") {
            mWeekDay = v;
        } else if(type == "h") {
            mHour = v;
        } else if(type == "m") {
            mMinute = v;
        }
    }
}