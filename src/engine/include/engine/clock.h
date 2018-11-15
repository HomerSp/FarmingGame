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
        int mYear;
        int mMonth;
        int mDay;
        int mWeek;
        int mWeekDay;
        int mHour;
        int mMinute;
    };

public:
    Clock();
 
    double current() const;
    uint32_t year() const;
    uint32_t month() const;
    uint32_t day() const;
    uint32_t week() const;
    uint32_t weekDay() const;
    uint32_t hour() const;
    uint32_t minute() const;

    uint32_t dawn() const;
    uint32_t sunrise() const;
    uint32_t sunset() const;
    uint32_t dusk() const;

    bool daylight() const;

    bool processAsync(float frameDiff, Map* map);
    void processListeners();

    void fastForward(float v) {
        mCurrent.store(mCurrent + v);
    }

    void setTime(int h, int m);

    // Scripting
    void on(const std::string& type, const std::string& format, asIScriptFunction* func);

    static void registerClass(asIScriptEngine* engine);
    static std::string className();

private:
    std::atomic<double> mCurrent;
    uint32_t mDawn;
    uint32_t mSunrise;
    uint32_t mSunset;
    uint32_t mDusk;

    std::mutex mListenerMutex;
    std::vector<std::shared_ptr<ChangeListener>> mChangeListeners;
};
} 
