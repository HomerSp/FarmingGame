#pragma once

#include <engine/context.h>
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
        Tool = 0,
        LightSource,

        Last,
    } Type;
};

class Item : public ContextObject, public ScreenEffects::LightSource {
public:
    Item(std::shared_ptr<Context>& ctx, const std::string& name);
    virtual ~Item() {}

    // Light source
    virtual Types::Point<int32_t> lightPosition();
    virtual int32_t lightRadius();
    virtual float_t lightStrength();

    void use(Player& player);

    const Image& uiImage() const;

private:
    std::unique_ptr<Image> mUiImage;

    int32_t mLightRadius;
    float_t mLightStrength;

    std::unordered_map<ItemEffect::Type, std::shared_ptr<ItemValue>> mEffects;
    std::bitset<ItemAttribute::Last> mAttributes;
};
}