#pragma once

#include <engine/screeneffects.h>

namespace engine {

namespace graphics {
class Image;
}

class Context;
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

class Item : public ContextObject, public Overlay::LightSource {
public:
    Item(std::shared_ptr<Context>& ctx, const std::string& name);
    virtual ~Item() {}

    // Light source
    virtual Types::Point<int32_t> lightPosition() override;
    virtual int32_t lightRadius() override;
    virtual graphics::ColorGradient lightColor() override;

    void use(Player& player);

    const graphics::Image& uiImage() const;

private:
    std::unique_ptr<graphics::Image> mUiImage;

    int32_t mLightRadius;
    graphics::ColorGradient mLightColor;

    std::unordered_map<ItemEffect::Type, std::shared_ptr<ItemValue>> mEffects;
    std::bitset<ItemAttribute::Last> mAttributes;
};
}