#pragma once

#include <atomic>
#include <cmath>
#include <mutex>
#include <unordered_map>

#include <engine/listeners.h>
#include <engine/scriptobject.h>
#include <engine/types.h>

namespace engine {

class Map;

class Clock : public ScriptObject {
private:
    class ChangeListener : public Listeners::Listener {
    public:
        ChangeListener(asIScriptFunction* fun, const std::string& format);

        bool check(uint64_t val);

    protected:
        void parseBlock(const std::string& block);

    private:
        bool mTriggered;
        int8_t mYear;
        int8_t mMonth;
        int8_t mDay;
        int8_t mWeek;
        int8_t mWeekDay;
        int8_t mHour;
        int8_t mMinute;
    };

public:
    Clock();
 
    uint64_t current() const;
    uint8_t year() const;
    uint8_t month() const;
    uint8_t day() const;
    uint8_t week() const;
    uint8_t weekDay() const;
    uint8_t hour() const;
    uint8_t minute() const;

    uint8_t dawn() const;
    uint8_t sunrise() const;
    uint8_t sunset() const;
    uint8_t dusk() const;

    bool daylight() const;

    std::string hourFormatted() const;
    std::string minuteFormatted() const;

    bool processAsync(uint64_t frameDiff, Map* map);
    void processListeners();

    void fastForward(float_t v);
    void setTime(int32_t h, int32_t m);

    // Scripting
    void on(const std::string& type, const std::string& format, asIScriptFunction* func);

    static void registerClass(asIScriptEngine* engine);
    static std::string className();

private:
    std::atomic<uint64_t> mCurrent;
    float_t mCurrentMod;
    uint8_t mDawn;
    uint8_t mSunrise;
    uint8_t mSunset;
    uint8_t mDusk;

    std::mutex mListenerMutex;
    std::vector<std::shared_ptr<ChangeListener>> mChangeListeners;
};
} 
