#pragma once

#include <cmath>
#include <unordered_map>

#include <engine/listeners.h>
#include <engine/renderer.h>
#include <engine/scriptobject.h>
#include <engine/types.h>

namespace engine {
class Clock : public ScriptObject {
private:
    class ChangeListener : public Listeners::Listener {
    public:
        ChangeListener(asIScriptFunction* fun, const std::string& format, bool once = false);

        bool check(asIScriptContext& ctx, uint64_t val);

        bool once() const {
            return mOnce;
        }

    protected:
        void parseBlock(const std::string& block);

    private:
        bool mOnce;
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
 
    uint32_t year() const;
    uint32_t month() const;
    uint32_t day() const;
    uint32_t week() const;
    uint32_t weekDay() const;
    uint32_t hour() const;
    uint32_t minute() const;

    void draw(Renderer& renderer);
    void process(uint64_t frameDiff);

    void fastForward(float v) {
        mCurrent += v;
    }

    void setTime(int h, int m);

    // Scripting
    void on(const std::string& type, const std::string& format, asIScriptFunction* func);

protected:
    virtual std::string className();
    virtual void registerClass();

private:
    double mCurrent;
    uint32_t mDawn;
    uint32_t mSunrise;
    uint32_t mSunset;
    uint32_t mDusk;

    std::vector<std::shared_ptr<ChangeListener>> mChangeListeners;
};
} 
