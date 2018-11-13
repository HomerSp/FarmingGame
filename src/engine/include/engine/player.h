#pragma once

#include <engine/character.h>
#include <engine/clock.h>

namespace engine {
class Player : public Character, public Clock::LightSource {
public:
    Player();

    virtual Types::Point<int32_t> light();
    virtual int32_t radius();
    virtual float strength();

    Character* character();

    static void registerClass(asIScriptEngine* engine);
    static std::string className();
};
}