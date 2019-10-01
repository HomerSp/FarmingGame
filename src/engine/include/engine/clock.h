#pragma once

#include <atomic>
#include <cmath>
#include <mutex>
#include <unordered_map>

#include <engine/listeners.h>
#include <engine/listenerobject.h>
#include <engine/script/scriptobject.h>
#include <engine/time.h>
#include <engine/types.h>

namespace engine {

class Context;
class Map;

class Clock : public script::ScriptObject {
public:
    typedef enum {
        Spring = 0,
        Summer,
        Autumn,
        Winter,
        Last = Winter,
    } Month;

private:
    class ChangeListener : public Listeners::Listener {
    public:
        ChangeListener(asIScriptFunction* fun, const std::string& format);

        bool check(uint64_t val);

    private:
        bool mTriggered;
        Time mTime;
    };

public:
    Clock(std::shared_ptr<Context> &ctx);

    float_t currentHour() const;
    uint64_t current() const;

    uint8_t year() const;
    uint8_t month() const;
    uint8_t day() const;
    uint8_t week() const;
    uint8_t weekDay() const;
    uint8_t hour() const;
    uint8_t minute() const;
    uint8_t minuteRounded() const;

    uint8_t yearDisplay() const;
    uint8_t monthDisplay() const;
    uint8_t dayDisplay() const;
    uint8_t weekDisplay() const;
    uint8_t weekDayDisplay() const;

    // Used for calculating screen overlay colour and opacity.
    uint8_t dawn() const;
    uint8_t sunrise() const;
    uint8_t sunset() const;
    uint8_t dusk() const;

    bool daylight() const;

    std::string timeFormatted() const;
    std::string dayFormatted() const;
    std::string weekDayFormattedShort() const;

    bool processAsync(uint64_t frameDiff, Map* map);
    void processListeners();

    void fastForward(float_t v);
    void setTime(int32_t h, int32_t m);

    // Scripting
    void on(const std::string& type, const std::string& format, asIScriptFunction* func);

    static void registerClass(asIScriptEngine* engine);
    static std::string className();

public:
    ListenerObject DayChanged;
    ListenerObject DaylightChanged;

protected:
    uint8_t sunMonthMod() const;

private:
    std::atomic<uint64_t> mCurrent;
    Types::AtomicF mCurrentMod;
    uint8_t mDawn;
    uint8_t mDusk;

    std::mutex mListenerMutex;
    std::vector<std::shared_ptr<ChangeListener>> mChangeListeners;

    std::atomic<bool> mChangedDay, mChangedDaylight;
};
} 
