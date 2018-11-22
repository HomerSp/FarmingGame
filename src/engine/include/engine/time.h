#pragma once

#include <unordered_map>

namespace engine {
class Time {
public:
    typedef enum {
        Year = 0,
        Month,
        Day,
        Week,
        WeekDay,
        Hour,
        Minute,
    } Type;

    static void parseString(const std::string& format, std::unordered_map<Type, int8_t>& out);

private:
    static void parseBlock(const std::string& block, std::unordered_map<Type, int8_t>& out);

};
}
