#pragma once

#include <cmath>
#include <string>
#include <unordered_map>

namespace engine {

class Clock;

class Time {
public:
    Time();

    int8_t year() const;
    int8_t month() const;
    int8_t day() const;
    int8_t week() const;
    int8_t weekDay() const;
    int8_t hour() const;
    int8_t minute() const;

    bool equals(const Time& time) const;

    static Time fromCurrent(uint64_t val);
    static Time fromString(const std::string& format);

private:
    void parseBlock(const std::string& block);

    int8_t mYear, mMonth, mDay, mWeek, mWeekDay, mHour, mMinute;

};
}
