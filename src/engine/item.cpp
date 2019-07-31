#include <cmath>
#include <memory>

#include <json/json.h>

#include <engine/assetmanager.h>
#include <engine/character/character.h>
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
    , mLightRadius(0)
    , mLightStrength(0.0f)
{
    std::unique_ptr<Json::Value> doc = AssetManager::get()->data(AssetManager::Item, name);
    if (!doc->isObject() || !doc->isMember("image")) {
        Logger::critical() << "Invalid JSON data for item" << name;
        return;
    }

    Json::Value imageObj = (*doc)["image"];
    if (!imageObj.isObject() || !imageObj.isMember("item")) {
        Logger::critical() << "Invalid JSON image data for item" << name;
        return;
    }

    mUiImage = AssetManager::get()->image(AssetManager::Item, imageObj["item"].asString());

    if (doc->isMember("attributes")) {
        Json::Value attrsObj = (*doc)["attributes"];

        for (const auto& attrObj : attrsObj) {
            std::string key = attrObj.asString();
            switch (Types::hash(key.c_str())) {
            case Types::hash("tool"):
                mAttributes[ItemAttribute::Tool] = true;
                break;
            case Types::hash("light_source"):
                mAttributes[ItemAttribute::LightSource] = true;
                break;
            default:
                Logger::warning() << "Unknown attribute" << key << "for item" << name;
                break;
            }
        }
    }

    if (mAttributes[ItemAttribute::LightSource] && doc->isMember("light")) {
        Json::Value lightObj = (*doc)["light"];
        if (lightObj.isMember("radius")) {
            mLightRadius = lightObj["radius"].asInt();
        }

        if (lightObj.isMember("strength")) {
            mLightStrength = lightObj["strength"].asInt() / 100.0f;
        }
    }

    if (doc->isMember("use")) {
        Json::Value useObj = (*doc)["use"];
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

Types::Point<int32_t> Item::lightPosition()
{
    return {0, 0};
}

int32_t Item::lightRadius()
{
    return mLightRadius;
}

float_t Item::lightStrength()
{
    return mLightStrength;
}

Types::Color Item::lightColor()
{
    return {255, 100, 0, 175};
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

const Image& Item::uiImage() const
{
    return *mUiImage;
}