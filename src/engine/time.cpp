#include <engine/time.h>

using namespace engine;


void Time::parseString(const std::string& format, std::unordered_map<Type, int8_t>& out)
{
    size_t start = 0;
    if(format.find(',') != std::string::npos) {
        size_t end = 0;
        while((end = format.find(',', end)) != std::string::npos)
        {
            parseBlock(format.substr(start, end), out);
            end++;
            start = end;
        }
    }

    if (start != std::string::npos) {
        parseBlock(format.substr(start), out);
    }
}

void Time::parseBlock(const std::string& block, std::unordered_map<Type, int8_t>& out)
{
    size_t end = 0;
    auto v = static_cast<int8_t>(std::stoi(block, &end, 10));
    if (end < block.size()) {
        std::string type = block.substr(end);
        if(type == "y") {
            out[Year] = v;
        } else if(type == "mon") {
            out[Month] = v;
        } else if(type == "d") {
            out[Day] = v;
        } else if(type == "w") {
            out[Week] = v;
        } else if(type == "wd") {
            out[WeekDay] = v;
        } else if(type == "h") {
            out[Hour] = v;
        } else if(type == "m") {
            out[Minute] = v;
        }
    }
}
