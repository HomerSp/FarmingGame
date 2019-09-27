#pragma once

#include <memory>
#include <unordered_map>

#include <json/value.h>

#include <engine/contextobject.h>
#include <engine/graphics/image.h>
#include <engine/graphics/renderer.h>
#include <engine/types.h>

namespace engine {

namespace graphics {
class BufferWriter;
}

class Context;

struct CharsetNode {
public:
    CharsetNode(const Types::Rect<>& rc, const Types::Cells& cells, const Types::Rect<>& collision);

    Types::Rect<> rect;
    Types::Cells cells;
    Types::Rect<> collision;
};

class Charset : public ContextObject {
public:
    enum Type {
        TypeWalk = 0,
    };

    Charset(std::shared_ptr<Context>& ctx, const std::string& name);

    void updateBuffer(graphics::Renderer& renderer, const Types::Point<>& pos, Charset::Type type, int32_t direction, int32_t frame, graphics::Buffer::Writer& writer, uint32_t texture, float_t zOrder);

    graphics::Image& image() const;

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
    std::unique_ptr<graphics::Image> mImage;
    std::unordered_map<Charset::Type, std::shared_ptr<CharsetNode>> mNodes;
};
}
