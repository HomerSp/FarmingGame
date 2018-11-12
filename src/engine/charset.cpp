#include <json/json.h>

#include <engine/assetmanager.h>
#include <engine/charset.h>
#include <engine/logger.h>

using namespace engine;

CharsetNode::CharsetNode(const Types::Rect<>& rc, const Types::Cells& cells, const Types::Rect<>& collision)
    : rect(rc)
    , cells(cells)
    , collision(collision)
{
}

Charset::Charset(const std::string& name)
    : mValid(false)
    , mImage(nullptr)
{
    std::shared_ptr<Json::Value> docPtr = AssetManager::get()->data(AssetManager::get()->Charset, name);
    Json::Value& doc = *docPtr;
    if (!doc.isObject()) {
        Logger::critical() << "Could not open charset JSON file" << name;
        return;
    }

    if (!doc.isMember("image")) {
        Logger::critical() << "Could not find required charset JSON data for" << name;
        return;
    }

    mImage = AssetManager::get()->image(AssetManager::get()->Charset, doc["image"].asString());
    if (!*mImage) {
        Logger::critical() << "Could not load charset image for" << name;
        return;
    }

    if (doc.isMember("walk")) {
        addNode(Charset::TypeWalk, doc["walk"]);
    }

    mValid = !mNodes.empty();
}

void Charset::draw(Renderer& renderer, const Types::Point<>& pos, Charset::Type type, int direction, int frame)
{
    // Couldn't find the node, return...
    if (mNodes.find(type) == mNodes.end()) {
        return;
    }

    std::shared_ptr<CharsetNode> node = mNodes.find(type)->second;
    Types::Rect<> dst(pos.x, pos.y, node->rect.width, node->rect.height);
    Types::Rect<> src(node->rect.x + (node->rect.width * frame), node->rect.y + (node->rect.height * direction), node->rect.width, node->rect.height);
    renderer.drawImage(*mImage, dst, src);
}

int Charset::width(Charset::Type type)
{
    if (mNodes.find(type) == mNodes.end()) {
        return 0;
    }

    return mNodes.at(type)->rect.width;
}

int Charset::height(Charset::Type type)
{
    if (mNodes.find(type) == mNodes.end()) {
        return 0;
    }

    return mNodes.at(type)->rect.height;
}

int Charset::columns(Charset::Type type)
{
    if (mNodes.find(type) == mNodes.end()) {
        return 0;
    }

    return mNodes.find(type)->second->cells.cols;
}

Types::Rect<> Charset::collision(Charset::Type type)
{
    if (mNodes.find(type) == mNodes.end()) {
        return Types::Rect<>();
    }

    return mNodes.find(type)->second->collision;
}

void Charset::addNode(Charset::Type type, Json::Value& val)
{
    if (!val.isMember("size") || !val.isMember("pos") || !val.isMember("cells")) {
        Logger::error() << "Could not find required json values for charset";
        return;
    }

    Json::Value size = val["size"];
    Json::Value pos = val["pos"];
    Json::Value cells = val["cells"];
    Json::Value collision = val["collision"];
    if (size.size() != 2 || pos.size() != 2 || cells.size() != 2 || collision.size() != 4) {
        Logger::error() << "Invalid json values for charset";
        return;
    }

    Types::Rect<> rect(pos[0].asInt(), pos[1].asInt(), size[0].asInt(), size[1].asInt());
    Types::Cells c(cells[0].asInt(), cells[1].asInt());
    Types::Rect<> col(collision[0].asInt(), collision[1].asInt(), collision[2].asInt(), collision[3].asInt());
    mNodes[type] = std::make_shared<CharsetNode>(rect, c, col);
}
