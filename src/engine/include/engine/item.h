#pragma once

#include <engine/screeneffects.h>

namespace engine {

class Image;
class Player;

struct ItemValue {
public:
    ItemValue(bool percent, int32_t value);
    
    bool percent;
    int32_t value;
};

struct ItemEffect {
public:
    typedef enum {
        Unknown,
        Stamina,
        Health,
    } Type;
};

struct ItemAttribute {
    typedef enum {
        Consume = 0,
        LightSource,

        Last,
    } Type;
};

class Item : public ScreenEffects::LightSource {
public:
    Item(const std::string& name);

    // Light source
    virtual Types::Point<int32_t> lightPosition();
    virtual int32_t lightRadius();
    virtual float_t lightStrength();
    virtual Types::Color lightColor();

    void use(Player& player);

    std::shared_ptr<Image> uiImage();

private:
    std::shared_ptr<Image> mUiImage;

    int32_t mLightRadius;
    float_t mLightStrength;

    std::unordered_map<ItemEffect::Type, std::shared_ptr<ItemValue>> mEffects;
    std::bitset<ItemAttribute::Last> mAttributes;
};
}