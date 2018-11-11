#include <engine/player.h>

using namespace engine;

Player::Player()
    : Character("player")
{
}

Character* Player::character()
{
    return this;
}

void Player::registerClass(asIScriptEngine* engine)
{
    registerReference<Player>(engine);
    REGISTER_FUNC(engine, Player, Character&, character);
}

std::string Player::className()
{
    return "Player";
}
