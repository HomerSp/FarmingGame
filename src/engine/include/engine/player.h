#pragma once

#include <engine/character.h>
#include <engine/screeneffects.h>

namespace engine {
class Player : public Character, public ScreenEffects::LightSource {
public:
    Player();

    virtual Types::Point<int32_t> position();
    virtual int32_t radius();
    virtual float strength();

    Character* character();

    static void registerClass(asIScriptEngine* engine);
    static std::string className();
};
}