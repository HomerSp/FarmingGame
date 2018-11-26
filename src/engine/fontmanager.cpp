#include <json/json.h>

#include <engine/assetmanager.h>
#include <engine/fontmanager.h>
#include <engine/logger.h>

using namespace engine;

std::shared_ptr<FontManager> FontManager::sInstance = nullptr;

std::shared_ptr<FontManager> FontManager::get()
{
    if (!sInstance) {
        struct make_shared_enabler : public FontManager {};
        sInstance = std::make_shared<make_shared_enabler>();
    }

    return sInstance;
}

FontManager::FontManager()
{
    std::shared_ptr<Json::Value> docPtr = AssetManager::get()->data(AssetManager::Font, "types");
    Json::Value doc = *docPtr;
    if (!doc.isObject()) {
        Logger::critical() << "Invalid JSON data for font";
        return;
    }

    for (auto it = doc.begin(); it != doc.end(); it++) {
        mFonts.emplace(it.name(), it->asString());
    }

    if (mFonts.find("default") == mFonts.end()) {
        Logger::warning() << "Missing default font";
    }
}

bool FontManager::files(std::vector<std::string>& out)
{
    std::shared_ptr<Json::Value> docPtr = AssetManager::get()->data(AssetManager::Font, "files");
    Json::Value doc = *docPtr;
    if (doc.empty()) {
        Logger::critical() << "Invalid JSON data for font files";
        return false;
    }

    std::shared_ptr<AssetManager> am = AssetManager::get();
    for (Json::Value& v: doc) {
        out.emplace_back(am->fontPath(v.asString()));
    }

    return true;
}

std::string FontManager::font(const std::string &type)
{
    if (mFonts.find(type) == mFonts.end()) {
        return mFonts["default"];
    }

    return mFonts[type];
}