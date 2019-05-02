#include <fstream>

#include <engine/assetmanager.h>
#include <engine/collisionmap.h>
#include <engine/image.h>
#include <engine/logger.h>
#include <engine/renderer.h>

using namespace engine;

std::shared_ptr<AssetManager> AssetManager::sInstance = nullptr;

AssetManager::AssetManager()
    : mBase("assets")
{
}

std::shared_ptr<AssetManager> AssetManager::get()
{
    if (!sInstance) {
        struct make_shared_enabler : public AssetManager {};
        sInstance = std::make_shared<make_shared_enabler>();
    }

    return sInstance;
}

std::shared_ptr<Json::Value> AssetManager::data(Type type, const std::string& name)
{
    std::shared_ptr<Json::Value> doc = std::make_shared<Json::Value>();

    std::string path = AssetManager::dataPath(type, name);
    if (path.length() > 0) {
        std::ifstream file(path);
        try {
            file >> *doc.get();
        } catch(const Json::RuntimeError &e) {
            Logger::critical() << "Could not parse file" << path << e.what();
        }
    }

    return doc;
}

std::shared_ptr<engine::Image> AssetManager::image(Type type, const std::string& name)
{
    std::string path = AssetManager::imagePath(type, name);
    if (path.length() > 0) {
        return mRenderer->loadImage(path);
    }

    return nullptr;
}

std::shared_ptr<engine::CollisionMap> AssetManager::collision(Type type, const std::string& name)
{
    std::string path = AssetManager::collisionPath(type, name);
    if (path.length() > 0) {
        return std::make_shared<engine::CollisionMap>(path);
    }

    return std::make_shared<engine::CollisionMap>();
}

std::string AssetManager::dataPath(Type type, const std::string& name)
{
    std::string ret = mBase + "/data/";
    switch (type) {
    case Character:
        ret += "character";
        break;
    case Map:
        ret += "map";
        break;
    case Charset:
        ret += "charset";
        break;
    case Tileset:
        ret += "tileset";
        break;
    case Font:
        ret += "font";
        break;
    case Item:
        ret += "item";
        break;
    default:
        return "";
    }
    return ret + "/" + name + ".json";
}

std::string AssetManager::imagePath(Type type, const std::string& name)
{
    std::string ret = mBase + "/image/";
    switch (type) {
    case Charset:
        ret += "charset";
        break;
    case Item:
        ret += "item";
        break;
    case Tileset:
        ret += "tileset";
        break;
    case Ui:
        ret += "ui";
        break;
    default:
        return "";
    }
    return ret + "/" + name + ".png";
}

std::string AssetManager::collisionPath(Type type, const std::string& name)
{
    std::string ret = mBase + "/collision/";
    switch (type) {
    case Tileset:
        ret += "tileset";
        break;
    default:
        return "";
    }
    return ret + "/" + name + ".png";
}

std::string AssetManager::fontPath(const std::string& name)
{
    return mBase + "/font/" + name;
}

void AssetManager::setRenderer(std::shared_ptr<Renderer> renderer)
{
    mRenderer = std::move(renderer);
}