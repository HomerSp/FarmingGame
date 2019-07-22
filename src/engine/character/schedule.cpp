#include <json/json.h>

#include <engine/character/schedule.h>
#include <engine/logger.h>

using namespace engine::character;

Schedule::Schedule(const std::string& name)
{
    Logger::debug() << "Schedule" << name;
}
