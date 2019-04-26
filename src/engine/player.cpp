#include <engine/assetmanager.h>
#include <engine/character.h>
#include <engine/item.h>
#include <engine/logger.h>
#include <engine/player.h>

using namespace engine;

Player::Player()
    : Character("player")
    , mControl(true)
    , mLevel(1)
    , mStamina(0)
    , mHealth(0)
    , mCurrentItem(0)
{
    std::shared_ptr<Json::Value> docPtr = AssetManager::get()->data(AssetManager::get()->Character, "player");
    Json::Value doc = *docPtr;
    if (!doc.isObject() || !doc.isMember("stats")) {
        Logger::critical() << "Invalid JSON data for player";
        return;
    }

    Json::Value statsObj = doc["stats"];
    if (!statsObj.isMember("max_level") || ! statsObj.isMember("stamina") || !statsObj.isMember("health")) {
        Logger::critical() << "Missing JSON stats data for player";
        return;
    }

    Json::Value level = statsObj["max_level"];
    mStats.maxLevel = static_cast<int8_t>(level.asInt());

    Json::Value stamina = statsObj["stamina"];
    mStats.stamina = std::make_pair(stamina[0].asInt(), stamina[1].asInt());

    Json::Value health = statsObj["health"];
    mStats.health = std::make_pair(health[0].asInt(), health[1].asInt());

    mStamina = mStats.currentStamina(mLevel);
    mHealth = mStats.currentHealth(mLevel);

    mInventory[0] = std::make_shared<Item>("tool_axe_1");
    mInventory[1] = std::make_shared<Item>("tool_hammer_1");
    mInventory[2] = std::make_shared<Item>("tool_torch_1");
    mInventory[3] = std::make_shared<Item>("vial_potion");
    mInventory[4] = std::make_shared<Item>("fruit_apple");
    mInventory[5] = std::make_shared<Item>("fruit_rotten_apple");
    mInventory[6] = std::make_shared<Item>("vial_poison");
    mInventory[7] = std::make_shared<Item>("vial_potion");
}

Types::Point<int32_t> Player::lightPosition()
{
    return position();
}

int32_t Player::lightRadius()
{
    return mInventory[mCurrentItem]->lightRadius();
}

float_t Player::lightStrength()
{
    return mInventory[mCurrentItem]->lightStrength();
}

Types::Color Player::lightColor()
{
    return mInventory[mCurrentItem]->lightColor();
}

Types::Point<int32_t> Player::position()
{
    return {static_cast<int32_t>(x() + width() / 2), static_cast<int32_t>(y() + height() * 0.75f)};
}

bool Player::canControl() const
{
    return mControl;
}

void Player::setControl(bool control)
{
    mControl = control;
}

uint16_t Player::stamina()
{
    return mStamina;
}

uint16_t Player::maxStamina()
{
    return mStats.currentStamina(mLevel);
}

uint16_t Player::health()
{
    return mHealth;
}

uint16_t Player::maxHealth()
{
    return mStats.currentHealth(mLevel);
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

void Player::decrementItem()
{
    if (mCurrentItem == 0) {
        mCurrentItem = mInventory.size();
    }

    mCurrentItem--;
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

int32_t Player::Stats::currentStamina(int8_t level)
{
    level -= 1;
    return stamina.first + (((stamina.second - stamina.first) / maxLevel) * level);
}

int32_t Player::Stats::currentHealth(int8_t level)
{
    level -= 1;
    return health.first + (((health.second - health.first) / maxLevel) * level);
}
