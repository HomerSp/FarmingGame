#include <engine/time.h>

using namespace engine;

Time::Time()
    : mYear(-1)
    , mMonth(-1)
    , mDay(-1)
    , mWeek(-1)
    , mWeekDay(-1)
    , mHour(-1)
    , mMinute(-1)
{
}

int8_t Time::year() const
{
    return mYear;
}

int8_t Time::month() const
{
    return mMonth;
}

int8_t Time::day() const
{
    return mDay;
}

int8_t Time::week() const
{
    return mWeek;
}

int8_t Time::weekDay() const
{
    return mWeekDay;
}

int8_t Time::hour() const
{
    return mHour;
}

int8_t Time::minute() const
{
    return mMinute;
}

bool Time::equals(const Time& time) const
{
    if (mYear != -1 && mYear != time.year()) {
        return false;
    }

    if (mMonth != -1 && mMonth != time.month()) {
        return false;
    }

    if (mDay != -1 && mDay != time.day()) {
        return false;
    }

    if (mWeek != -1 && mWeek != time.week()) {
        return false;
    }

    if (mWeekDay != -1 && mWeekDay != time.weekDay()) {
        return false;
    }

    if (mHour != -1 && mHour != time.hour()) {
        return false;
    }

    if (mMinute != -1 && mMinute != time.minute()) {
        return false;
    }

    return true;
}

Time Time::fromCurrent(uint64_t val)
{
    Time ret;
    ret.mMinute = static_cast<int8_t>(val % 60);
    val /= 60;
    ret.mHour = static_cast<int8_t>(val % 24);
    val /= 24;
    ret.mWeekDay = 1 + (static_cast<int8_t>(val % 7));
    ret.mDay = 1 + (static_cast<int8_t>(val % 28));
    ret.mWeek = 1 + (static_cast<int8_t>(val / 7 % 4));
    val /= 28;
    ret.mMonth = 1 + (static_cast<int8_t>(val % 4));
    val /= 4;
    ret.mYear = 1 + (static_cast<int8_t>(val));
    return ret;
}

Time Time::fromString(const std::string& format)
{
    Time ret;

    size_t start = 0;
    if(format.find(',') != std::string::npos) {
        size_t end = 0;
        while((end = format.find(',', end)) != std::string::npos)
        {
            ret.parseBlock(format.substr(start, end));
            end++;
            start = end;
        }
    }

    if (start != std::string::npos) {
        ret.parseBlock(format.substr(start));
    }

    return ret;
}

void Time::parseBlock(const std::string& block)
{
    size_t end = 0;
    auto v = static_cast<int8_t>(std::stoi(block, &end, 10));
    if (end < block.size()) {
        std::string type = block.substr(end);
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
