#pragma once

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

class Item {
public:
    Item(const std::string& name);

    void use(Player& player);

    std::shared_ptr<Image> uiImage();

private:
    std::shared_ptr<Image> mUiImage;
    std::unordered_map<ItemEffect::Type, std::shared_ptr<ItemValue>> mEffects;
};
}