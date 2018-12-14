#pragma once

#include <unordered_map>

#include <engine/screeneffects.h>

namespace engine {

class Character;
class Item;

class Player : public Character, public ScreenEffects::LightSource {
public:
    Player();

    virtual Types::Point<int32_t> position();
    virtual int32_t radius();
    virtual float_t strength();

    bool canControl() const;
    void setControl(bool control);

    void useItem();
    void incrementItem();
    void decrementItem();

    uint16_t stamina();
    uint16_t maxStamina();

    uint16_t health();
    uint16_t maxHealth();

    std::shared_ptr<Item> currentItem();

    const std::unordered_map<uint8_t, std::shared_ptr<Item>>& items();
    uint8_t currentItemIndex();

    Character* character();

    static void registerClass(asIScriptEngine* engine);
    static std::string className();

private:
    struct Stats {
        int32_t currentStamina(int8_t level);
        int32_t currentHealth(int8_t level);

        int8_t maxLevel;
        std::pair<int32_t, int32_t> stamina;
        std::pair<int32_t, int32_t> health;
    };

private:
    friend class Item;

    bool mControl;

    int8_t mLevel;
    Stats mStats;
    uint16_t mStamina, mHealth;

    std::unordered_map<uint8_t, std::shared_ptr<Item>> mInventory;
    uint8_t mCurrentItem;
};
}