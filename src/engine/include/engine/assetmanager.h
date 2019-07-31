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

    static std::shared_ptr<AssetManager> get();

    std::unique_ptr<Json::Value> data(Type type, const std::string& name);
    std::unique_ptr<Image> image(Type type, const std::string& name);
    std::unique_ptr<CollisionMap> collision(Type type, const std::string& name);

    std::string dataPath(Type type, const std::string& name);
    std::string imagePath(Type type, const std::string& name);
    std::string collisionPath(Type type, const std::string& name);
    std::string fontPath(const std::string& name);

    void setRenderer(std::shared_ptr<Renderer> renderer);

protected:
    AssetManager();

private:
    static std::shared_ptr<AssetManager> sInstance;

    std::string mBase;
    std::shared_ptr<Renderer> mRenderer;
};
}
