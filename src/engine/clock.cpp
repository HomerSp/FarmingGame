#include <engine/logger.h>
#include <engine/clock.h>

using namespace engine;

ClockListenerArg::ClockListenerArg()
    : triggered(false)
    , year(-1)
    , month(-1)
    , day(-1)
    , hour(-1)
    , minute(-1)
{
}

bool ClockListenerArg::operator==(double val)
{
    if (year != -1) {
        int v = static_cast<int>(std::floor(val / 60 / 24 / 30 / 4));
        if (year != v) {
            triggered = false;
            return false;
        }
    }

    if (month != -1) {
        int v = static_cast<int>(std::floor(val / 60 / 24 / 30)) % 4;
        if (month != v) {
            triggered = false;
            return false;
        }
    }

    if (day != -1) {
        int v = static_cast<int>(std::floor(val / 60 / 24)) % 30;
        if (day != v) {
            triggered = false;
            return false;
        }
    }

    if (hour != -1) {
        int v = static_cast<int>(std::floor(val / 60)) % 24;
        if (hour != v) {
            triggered = false;
            return false;
        }
    }

    if (minute != -1) {
        int v = static_cast<int>(std::floor(val)) % 60;
        if (minute != v) {
            triggered = false;
            return false;
        }
    }

    triggered = true;
    return true;
}

void ClockListenerArg::construct(void* memory)
{
    new(memory) ClockListenerArg();
}

void ClockListenerArg::destruct(void* memory)
{
    ((ClockListenerArg*) memory)->~ClockListenerArg();
}

std::string ClockListenerArg::className()
{
    return "ClockListenerArg";
}

void ClockListenerArg::registerClass()
{
    ScriptObject::registerClass(sizeof(ClockListenerArg), asFUNCTION(ClockListenerArg::construct), asFUNCTION(ClockListenerArg::destruct));
    registerProperty("int hour", asOFFSET(ClockListenerArg, hour));
    registerProperty("int minute", asOFFSET(ClockListenerArg, minute));
}

Clock::Clock()
    : mCurrent(0.0f)
    , mDawn(6)
    , mSunrise(8)
    , mSunset(18)
    , mDusk(20)
{
}

Clock::~Clock()
{
    for(auto p: mChangeListeners) {
        p.second->release();
    }
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

void Clock::addListener(const std::string& type, ClockListenerArg& arg, asIScriptFunction* func)
{
    auto ptr = FunctionPtr<>::get("ClockChangeListener");
    (*ptr) = func;

    mChangeListeners.push_back(std::make_pair(arg, std::move(ptr)));
}

std::string Clock::className()
{
    return "Clock";
}

std::string Clock::globalInstance()
{
    return "clock";
}

void Clock::registerClass()
{
    registerMethod("uint year()", asMETHOD(Clock, year));
    registerMethod("uint month()", asMETHOD(Clock, month));
    registerMethod("uint day()", asMETHOD(Clock, day));
    registerMethod("uint hour()", asMETHOD(Clock, hour));
    registerMethod("uint minute()", asMETHOD(Clock, minute));

    ClockListenerArg a;
    registerType(a);

    registerCallback<>("ClockChangeListener");
    registerMethod("void on(const string &in, const ClockListenerArg &in, ClockChangeListener @cb)", asMETHOD(Clock, addListener));
}