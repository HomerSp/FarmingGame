#include <cstdlib>

#include <engine/logger.h>
#include <engine/clock.h>

using namespace engine;

ClockListenerArg::ClockListenerArg(const std::string& format)
    : mTriggered(false)
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

bool ClockListenerArg::operator==(double val)
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
    mTriggered = true;
    return !t;
}

void ClockListenerArg::parseBlock(const std::string& block)
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

Clock::Clock()
    : mCurrent(0.0f)
    , mDawn(6)
    , mSunrise(8)
    , mSunset(18)
    , mDusk(20)
{
}

void Clock::release()
{
    for(auto p: mChangeListeners) {
        p.second->release();
    }

    mChangeListeners.clear();
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
        for (auto &p: mChangeListeners) {
            if (p.first == mCurrent) {
                p.second->call(scriptContext());
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

void Clock::addListener(const std::string& type, const std::string& format, asIScriptFunction* func)
{
    auto ptr = FunctionPtr<>::get<ClockChangeListener>(func);
    mChangeListeners.push_back(std::make_pair(format, ptr));
}

std::string Clock::className()
{
    return "Clock";
}

void Clock::registerClass()
{
    registerMethod(FunctionPtrHelper::functionString<uint>("year"), asMETHOD(Clock, year));
    registerMethod(FunctionPtrHelper::functionString<uint>("month"), asMETHOD(Clock, month));
    registerMethod(FunctionPtrHelper::functionString<uint>("day"), asMETHOD(Clock, day));
    registerMethod(FunctionPtrHelper::functionString<uint>("week"), asMETHOD(Clock, week));
    registerMethod(FunctionPtrHelper::functionString<uint>("weekDay"), asMETHOD(Clock, weekDay));
    registerMethod(FunctionPtrHelper::functionString<uint>("hour"), asMETHOD(Clock, hour));
    registerMethod(FunctionPtrHelper::functionString<uint>("minute"), asMETHOD(Clock, minute));

    registerCallback<ClockChangeListener>();
    registerMethod(FunctionPtrHelper::functionString<void, const std::string&, const std::string&, ClockChangeListener&>("on"), asMETHOD(Clock, addListener));
}