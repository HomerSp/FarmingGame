#pragma once

#include <memory>
#include <string>

#include <json/json.h>
#include <scriptbuilder/scriptbuilder.h>

#include <engine/collisionmap.h>

namespace engine {

namespace graphics {
class Image;
class Renderer;
}

class AssetManager {
public:
    enum Type {
        Character = 0,
        Map,
        Charset,
        Tileset,
        Font,
        Ui,
        Item,
        Schedule,
        Config,
    };

    AssetManager();

    std::unique_ptr<Json::Value> data(Type type, const std::string& name) const;
    std::unique_ptr<graphics::Image> image(Type type, const std::string& name) const;
    std::unique_ptr<CollisionMap> collision(Type type, const std::string& name) const;
    std::string script(const std::string& name) const;

    std::string dataPath(Type type, const std::string& name) const;
    std::string imagePath(Type type, const std::string& name) const;
    std::string collisionPath(Type type, const std::string& name) const;
    std::string fontPath(const std::string& name) const;
    std::string scriptPath(const std::string& name) const;
    std::string shaderPath(const std::string& name, const std::string& ext) const;

private:
    std::string mBase;
};
}
