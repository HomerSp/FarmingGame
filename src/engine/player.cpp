#include <engine/character.h>
#include <engine/item.h>
#include <engine/player.h>

using namespace engine;

Player::Player()
    : Character("player")
    , mMaxStamina(500)
    , mMaxHealth(500)
    , mStamina(mMaxStamina)
    , mHealth(mMaxHealth)
    , mCurrentItem(0)
{
    mInventory[0] = std::make_shared<Item>("axe");
    mInventory[1] = std::make_shared<Item>("hammer");
    mInventory[2] = std::make_shared<Item>("sickle");
    mInventory[3] = std::make_shared<Item>("apple");
    mInventory[4] = std::make_shared<Item>("apple_rotten");
}

Types::Point<int32_t> Player::position()
{
    return {static_cast<int32_t>(x() + width() / 2), static_cast<int32_t>(y() + height() * 0.75f)};
}

int32_t Player::radius()
{
    return Character::width() * 4;
}

float_t Player::strength()
{
    return 0.25f;
}

uint16_t Player::stamina()
{
    return mStamina;
}

uint16_t Player::maxStamina()
{
    return mMaxStamina;
}

uint16_t Player::health()
{
    return mHealth;
}

uint16_t Player::maxHealth()
{
    return mMaxHealth;
}

void Player::useItem()
{
    if (mInventory.find(mCurrentItem) == mInventory.end()) {
        return;
    }

    mInventory[mCurrentItem]->use(*this);
}

void Player::incrementItem()
{
    mCurrentItem++;
    if (mCurrentItem >= mInventory.size()) {
        mCurrentItem = 0;
    }
}

std::shared_ptr<Item> Player::currentItem()
{
    return mInventory[mCurrentItem];
}

const std::unordered_map<uint8_t, std::shared_ptr<Item>>& Player::items()
{
    return mInventory;
}

uint8_t Player::currentItemIndex()
{
    return mCurrentItem;
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
