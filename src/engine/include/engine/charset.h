#pragma once

#include <memory>
#include <unordered_map>

#include <json/json.h>

#include <engine/image.h>
#include <engine/renderer.h>
#include <engine/types.h>

namespace engine {
struct CharsetNode {
public:
    CharsetNode(const Types::Rect<>& rc, const Types::Cells& cells, const Types::Rect<>& collision);

    Types::Rect<> rect;
    Types::Cells cells;
    Types::Rect<> collision;
};

class Charset {
public:
    enum Type {
        TypeWalk = 0,
    };

    Charset(const std::string& name);

    void draw(Renderer& renderer, const Types::Point<>& pos, Charset::Type type, int32_t direction = 0, int32_t frame = 0);

    int32_t width(Charset::Type type);
    int32_t height(Charset::Type type);

    int32_t columns(Charset::Type type);

    Types::Rect<> collision(Charset::Type type);

    bool operator!() const
    {
        return !mValid;
    }

protected:
    void addNode(Charset::Type type, Json::Value& val);

private:
    bool mValid;
    std::unique_ptr<engine::Image> mImage;
    std::unordered_map<Charset::Type, std::shared_ptr<CharsetNode>> mNodes;
};
}
