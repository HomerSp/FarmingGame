#pragma once

#include <unordered_map>

#include <engine/item.h>
#include <engine/screeneffects.h>

namespace engine {

namespace character {
class Character;
}

class Player : public character::Character, public Overlay::LightSource {
public:
    Player(std::shared_ptr<Context> &ctx);

    // Light source
    virtual Types::Point<int32_t> lightPosition();
    virtual int32_t lightRadius();
    virtual float_t lightStrength();
    virtual graphics::ColorGradient lightColor();

    Types::Point<int32_t> position();

    bool canControl() const;
    void setCanControl(bool control);

    void useItem();
    void incrementItem();
    void decrementItem();

    uint16_t stamina();
    uint16_t maxStamina();

    uint16_t health();
    uint16_t maxHealth();

    const Item& currentItem() const;
    uint8_t currentItemIndex() const;

    const Item& item(uint8_t i) const;
    bool hasItem(uint8_t i) const;

    character::Character* character();

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

    std::unordered_map<uint8_t, std::unique_ptr<Item>> mInventory;
    uint8_t mCurrentItem;
};
}