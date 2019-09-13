#include <engine/assetmanager.h>
#include <engine/charset.h>
#include <engine/context.h>
#include <engine/graphics/buffer.h>
#include <engine/graphics/bufferwriter.h>
#include <engine/graphics/texture.h>
#include <engine/graphics/vector.h>
#include <engine/logger.h>

using namespace engine;

CharsetNode::CharsetNode(const Types::Rect<>& rc, const Types::Cells& cells, const Types::Rect<>& collision)
    : rect(rc)
    , cells(cells)
    , collision(collision)
{
}

Charset::Charset(std::shared_ptr<Context>& ctx, const std::string& name)
    : ContextObject(ctx)
    , mValid(false)
    , mImage(nullptr)
{
    std::unique_ptr<Json::Value> doc = context().assetManager().data(AssetManager::Charset, name);
    if (!doc || !doc->isObject()) {
        Logger::critical("Charset") << "Could not open charset JSON file" << name;
        return;
    }

    if (!doc->isMember("image")) {
        Logger::critical("Charset") << "Could not find required charset JSON data for" << name;
        return;
    }

    mImage = context().assetManager().image(AssetManager::Charset, (*doc)["image"].asString());
    if (!*mImage) {
        Logger::critical("Charset") << "Could not load charset image for" << name;
        return;
    }

    if (doc->isMember("walk")) {
        addNode(Charset::TypeWalk, (*doc)["walk"]);
    }

    mValid = !mNodes.empty();
}

void Charset::updateBuffer(graphics::Renderer& renderer, const Types::Point<>& pos, Charset::Type type, int32_t direction, int32_t frame, graphics::BufferWriter& writer, uint32_t texture, float_t zOrder)
{
    // Couldn't find the node, return...
    if (mNodes.find(type) == mNodes.end()) {
        writer.skip(engine::graphics::Vector3D::Size() + engine::graphics::Vector4D::Size() + engine::graphics::Vector2D::Size() + sizeof(float_t));
        return;
    }

    const std::shared_ptr<CharsetNode> &node = mNodes.find(type)->second;
    Types::Rect<> dst(pos.x, pos.y, node->rect.width, node->rect.height);
    Types::Rect<> src(node->rect.x + (node->rect.width * frame), node->rect.y + (node->rect.height * direction), node->rect.width, node->rect.height);

    writer += engine::graphics::Vector3D(dst.x, dst.y, zOrder);
    writer += engine::graphics::Vector4D(src.x, src.y, src.width, src.height);
    writer += engine::graphics::Vector2D(src.width, src.height);
    writer += static_cast<float_t>(texture);
    writer.release();
}

graphics::Image& Charset::image() const
{
    return *mImage;
}

int32_t Charset::width(Charset::Type type)
{
    if (mNodes.find(type) == mNodes.end()) {
        return 0;
    }

    return mNodes.at(type)->rect.width;
}

int32_t Charset::height(Charset::Type type)
{
    if (mNodes.find(type) == mNodes.end()) {
        return 0;
    }

    return mNodes.at(type)->rect.height;
}

int32_t Charset::columns(Charset::Type type)
{
    if (mNodes.find(type) == mNodes.end()) {
        return 0;
    }

    return mNodes.find(type)->second->cells.cols;
}

Types::Rect<> Charset::collision(Charset::Type type)
{
    if (mNodes.find(type) != mNodes.end()) {
        return mNodes.find(type)->second->collision;
    }

    return {};
}

void Charset::addNode(Charset::Type type, Json::Value& val)
{
    if (!val.isMember("size") || !val.isMember("pos") || !val.isMember("cells")) {
        Logger::error("Charset") << "addNode, Could not find required json values for charset";
        return;
    }

    Json::Value size = val["size"];
    Json::Value pos = val["pos"];
    Json::Value cells = val["cells"];
    Json::Value collision = val["collision"];
    if (size.size() != 2 || pos.size() != 2 || cells.size() != 2 || collision.size() != 4) {
        Logger::error("Charset") << "addNode, Invalid json values for charset";
        return;
    }

    Types::Rect<> rect(pos[0].asInt(), pos[1].asInt(), size[0].asInt(), size[1].asInt());
    Types::Cells c(cells[0].asInt(), cells[1].asInt());
    Types::Rect<> col(collision[0].asInt(), collision[1].asInt(), collision[2].asInt(), collision[3].asInt());
    mNodes[type] = std::make_shared<CharsetNode>(rect, c, col);
}
