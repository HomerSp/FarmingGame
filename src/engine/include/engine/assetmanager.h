#pragma once

#include <json/json.h>
#include <memory>
#include <string>

#include <engine/collisionmap.h>

namespace engine {

class Image;
class Renderer;

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
    };

    AssetManager(const Renderer& renderer);

    std::unique_ptr<Json::Value> data(Type type, const std::string& name) const;
    std::unique_ptr<Image> image(Type type, const std::string& name) const;
    std::unique_ptr<CollisionMap> collision(Type type, const std::string& name) const;

    std::string dataPath(Type type, const std::string& name) const;
    std::string imagePath(Type type, const std::string& name) const;
    std::string collisionPath(Type type, const std::string& name) const;
    std::string fontPath(const std::string& name) const;

private:
    std::string mBase;
    const Renderer& mRenderer;
};
}
