#include <engine/player.h>

using namespace engine;

Player::Player()
    : Character("player")
{
}

Types::Point<int32_t> Player::position()
{
    return Types::Point<int32_t>(x() + width() / 2, y() + height() * 0.75f);
}

int32_t Player::radius()
{
    return Character::width() * 4;
}

float Player::strength()
{
    return 0.25f;
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
