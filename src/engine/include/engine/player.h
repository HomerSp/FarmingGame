#pragma once

#include <engine/character.h>

namespace engine {
class Player : public Character {
public:
    Player();

    Character* character();

    static void registerClass(asIScriptEngine* engine);
    static std::string className();
};
}