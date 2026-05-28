#include <fstream>

#include <engine/assetmanager.h>
#include <engine/collisionmap.h>
#include <engine/graphics/image.h>
#include <engine/graphics/renderer.h>
#include <engine/logger.h>

using namespace engine;

AssetManager::AssetManager()
    : mBase("assets")
{
}

std::unique_ptr<Json::Value> AssetManager::data(Type type, const std::string& name) const
{
    std::unique_ptr<Json::Value> doc = std::make_unique<Json::Value>();

    std::string path = AssetManager::dataPath(type, name);
    if (path.length() > 0) {
        std::ifstream file(path);
        if (!file.good()) {
            Logger::critical("AssetManager") << "data, File does not exist" << path;
            return nullptr;
        }

        try {
            file >> *doc;
        } catch(const Json::RuntimeError &e) {
            Logger::critical("AssetManager") << "data, Could not parse file" << path << e.what();
        }
    }

    return doc;
}

std::unique_ptr<graphics::Image> AssetManager::image(Type type, const std::string& name) const
{
    std::string path = AssetManager::imagePath(type, name);
    if (path.length() > 0) {
        return std::make_unique<graphics::Image>(path);
    }

    return nullptr;
}

std::unique_ptr<CollisionMap> AssetManager::collision(Type type, const std::string& name) const
{
    std::string path = AssetManager::collisionPath(type, name);
    if (path.length() > 0) {
        return std::make_unique<CollisionMap>(path);
    }

    return std::make_unique<CollisionMap>();
}

std::string AssetManager::script(const std::string& name) const
{
    std::string path = scriptPath(name);
    if (path.length() > 0) {
        std::ifstream file(path);
        
        std::stringstream buffer;
        buffer << file.rdbuf();
        return buffer.str();
    }

    return "";   
}


std::string AssetManager::dataPath(Type type, const std::string& name) const
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
    case Config:
        ret += "config";
        break;
    default:
        return "";
    }
    return ret + "/" + name + ".json";
}

std::string AssetManager::imagePath(Type type, const std::string& name) const
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

std::string AssetManager::collisionPath(Type type, const std::string& name) const
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

std::string AssetManager::fontPath(const std::string& name) const
{
    return mBase + "/font/" + name;
}

std::string AssetManager::scriptPath(const std::string& name) const
{
    return mBase + "/script/" + name + ".as";
}

std::string AssetManager::shaderPath(const std::string& name, const std::string& ext) const
{
    return mBase + "/shader/" + name + "." + ext;
}
