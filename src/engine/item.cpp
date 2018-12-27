#include <cmath>
#include <memory>

#include <json/json.h>

#include <engine/assetmanager.h>
#include <engine/character.h>
#include <engine/image.h>
#include <engine/item.h>
#include <engine/logger.h>
#include <engine/player.h>
#include <engine/types.h>

using namespace engine;

ItemValue::ItemValue(bool percent, int32_t value)
    : percent(percent)
    , value(value)
{
}

Item::Item(const std::string& name)
    : mUiImage(nullptr)
{
    std::shared_ptr<Json::Value> docPtr = AssetManager::get()->data(AssetManager::Item, name);
    Json::Value doc = *docPtr;
    if (!doc.isObject() || !doc.isMember("image")) {
        Logger::critical() << "Invalid JSON data for item" << name;
        return;
    }

    Json::Value imageObj = doc["image"];
    if (!imageObj.isObject() || !imageObj.isMember("ui")) {
        Logger::critical() << "Invalid JSON image data for item" << name;
        return;
    }

    mUiImage = AssetManager::get()->image(AssetManager::Ui, imageObj["ui"].asString());

    if (doc.isMember("use")) {
        Json::Value useObj = doc["use"];
        for (auto it = useObj.begin(); it != useObj.end(); it++) {
            ItemEffect::Type type = ItemEffect::Unknown;
            switch (Types::hash(it.name().c_str())) {
            case Types::hash("stamina"):
                type = ItemEffect::Stamina;
                break;
            case Types::hash("health"):
                type = ItemEffect::Health;
                break;
            default:
                Logger::warning() << "Unknown use type for item" << name;
                break;
            }

            if (type == ItemEffect::Unknown) {
                continue;
            }

            mEffects[type] = std::make_shared<ItemValue>(false, it->asInt());
        }
    }
}

void Item::use(Player& player)
{
    for (auto &v: mEffects) {
        uint16_t* current = nullptr, max = 0;
        switch(v.first) {
        case ItemEffect::Stamina:
            current = &player.mStamina;
            max = player.maxStamina();
            break;
        case ItemEffect::Health:
            current = &player.mHealth;
            max = player.maxHealth();
            break;
        default:
            break;
        }

        if (current == nullptr) {
            continue;
        }

        auto s = static_cast<int32_t>(*current);
        if (!v.second->percent) {
            s += v.second->value;
        } else {
            float_t f = v.second->value / 100.0f;
            s += static_cast<int32_t>(s * f);
        }

        if (s < 0) {
            s = 0;
        }

        if (s > max) {
            s = max;
        }

        *current = static_cast<uint16_t>(s);
    }
}

std::shared_ptr<Image> Item::uiImage()
{
    return mUiImage;
}