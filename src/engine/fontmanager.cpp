#include <json/value.h>

#include <engine/assetmanager.h>
#include <engine/context.h>
#include <engine/fontmanager.h>
#include <engine/logger.h>

using namespace engine;

FontManager::FontManager(Context& context)
    : mContext(context)
{
    auto doc = mContext.assetManager().data(AssetManager::Font, "types");
    if (!doc || !doc->isObject()) {
        Logger::critical() << "Invalid JSON data for font";
        return;
    }

    for (auto it = doc->begin(); it != doc->end(); it++) {
        mFonts.emplace(it.name(), it->asString());
    }

    if (mFonts.find("default") == mFonts.end()) {
        Logger::warning() << "Missing default font";
    }
}

bool FontManager::files(std::vector<std::string>& out)
{
    auto doc = mContext.assetManager().data(AssetManager::Font, "files");
    if (!doc || doc->empty()) {
        Logger::critical() << "Invalid JSON data for font files";
        return false;
    }

    for (Json::Value& v: *doc) {
        out.emplace_back(mContext.assetManager().fontPath(v.asString()));
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