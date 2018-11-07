#pragma once

#include <cmath>
#include <unordered_map>

#include <engine/renderer.h>
#include <engine/scriptobject.h>
#include <engine/types.h>

namespace engine {
struct ClockListenerArg : public ScriptObject {
    ClockListenerArg();

    bool operator==(double val);

    static void construct(void* memory);
    static void destruct(void* memory);

    bool triggered;
    int year;
    int month;
    int day;
    int hour;
    int minute;

    virtual std::string className();
    virtual void registerClass();
};

class Clock : public ScriptObject {
public:
    Clock();
    ~Clock();

    uint32_t year() const;
    uint32_t month() const;
    uint32_t day() const;
    uint32_t hour() const;
    uint32_t minute() const;

    void draw(Renderer& renderer);
    void process(uint64_t frameDiff);

    void fastForward(float v) {
        mCurrent += v;
    }

    void setTime(int h, int m);

    // Scripting
    void addListener(const std::string& type, ClockListenerArg& arg, asIScriptFunction* func);

    virtual std::string className();
    virtual std::string globalInstance();
    virtual void registerClass();

private:
    double mCurrent;
    uint32_t mDawn;
    uint32_t mSunrise;
    uint32_t mSunset;
    uint32_t mDusk;

    std::vector<std::pair<ClockListenerArg, std::shared_ptr<FunctionPtr<>>>> mChangeListeners;
};
} 
