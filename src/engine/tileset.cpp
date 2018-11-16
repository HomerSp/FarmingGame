#include <cmath>
#include <fstream>

#include <json/json.h>

#include <engine/assetmanager.h>
#include <engine/collisionmap.h>
#include <engine/image.h>
#include <engine/logger.h>
#include <engine/renderer.h>
#include <engine/tileset.h>

using namespace engine;

TilesetType::TilesetType(uint32_t index, Types::Dimension<>& tileDimension, const std::string& tileType, const std::bitset<TilesetAttribute::Last>& attrs, uint32_t x, uint32_t y, int frames, Types::Cells count, uint32_t base)
    : mValid(false)
    , mIndex(index)
    , mTileDimension(tileDimension)
    , mTileType(TileTypeSingle)
    , mAttributes(attrs)
    , mSize(x, y, tileDimension.width, tileDimension.height)
    , mFrames(frames)
    , mCount(count)
    , mBase(base)
{
    if (tileType == "automatic") {
        mTileType = TileTypeAuto;
    } else if (tileType == "automatic_horizontal") {
        mTileType = TileTypeAutoHoriz;
    } else if (tileType == "single") {
        mTileType = TileTypeSingle;
    } else {
        Logger::critical() << "Invalid tileset node tile" << tileType;
        return;
    }

    switch (mTileType) {
    case TileTypeAuto:
        mSize.width = tileDimension.width * 2;
        if (frames > 0) {
            mSize.width *= frames;
        }
        mSize.height = tileDimension.height * 3;
        break;
    case TileTypeAutoHoriz:
        mSize.width *= 2;
        if (frames > 0) {
            mSize.height *= frames;
        }
        break;
    default:
        mSize.width *= mCount.cols;
        mSize.height *= mCount.rows;
        break;
    }

    if (mAttributes[TilesetAttribute::Toggle]) {
        mSize.width *= 2;
    }

    if (!mAttributes[TilesetAttribute::AboveRow] && !mAttributes[TilesetAttribute::AboveAll]) {
        mAttributes[TilesetAttribute::AboveNone] = true;
    }

    mValid = true;
}

bool TilesetType::contains(const Types::Point<uint32_t>& other) const
{
    return (other.x >= mSize.x && other.x < mSize.x + mSize.width && other.y >= mSize.y && other.y < mSize.y + mSize.height);
}

bool TilesetType::hasAttribute(TilesetAttribute::Type type) const
{
    return mAttributes[type];
}

void TilesetType::setLightBase(Types::Point<> base)
{
    mLightBase = std::move(base);
}

void TilesetType::setLightRadius(int radius)
{
    mLightRadius = radius;
}

void TilesetType::setLightStrength(float strength)
{
    mLightStrength = strength;
}

uint32_t TilesetType::index() const
{
    return mIndex;
}

Types::Point<> TilesetType::lightBase() const
{
    return mLightBase;
}

int TilesetType::lightRadius() const
{
    return mLightRadius;
}

float TilesetType::lightStrength() const
{
    return mLightStrength;;
}

bool TilesetType::operator!() const
{
    return !mValid;
}

bool TilesetType::checkBase(Types::Map2D& tiles, TilesetAttribute::Type type, uint32_t x, uint32_t y)
{
    if (mBase > 0) {
        if (type == TilesetAttribute::AboveRow) {
            return (y >= mBase && tiles[x][y - mBase] == tiles[x][y]);
        }
        if(type == TilesetAttribute::AboveAll) {
            return (y < mBase || tiles[x][y - mBase] != tiles[x][y]);
        }
    }

    return false;
}

std::shared_ptr<TilesetNode> TilesetType::toNode(Types::Map2D& tiles, uint32_t x, uint32_t y, uint32_t width, uint32_t height)
{
    std::shared_ptr<TilesetNode> node = std::make_shared<TilesetNode>();
    node->anim = Types::Point<>((mTileType == TileTypeAuto) ? (mTileDimension.width * 2) : 0, (mTileType == TileTypeAutoHoriz) ? mTileDimension.height : 0);
    node->frames = mFrames;
    node->current = 0;
    node->toggleWidth = (mAttributes[TilesetAttribute::Toggle]) ? (mSize.width / 2) : 0;
    node->toggled = false;
    node->type = this;

    switch (mTileType) {
    case TileTypeSingle: {
        Types::Point<uint32_t> pos(mSize.x, mSize.y);
        for(uint32_t iy = 1; iy < mCount.rows; iy++) {
            if (y - iy >= 0) {
                if (tiles[x][y - iy] == tiles[x][y]) {
                    pos.y += mTileDimension.height;
                }
            }
        }
        for(uint32_t ix = 1; ix < mCount.cols; ix++) {
            if (x - ix >= 0) {
                if (tiles[x - ix][y] == tiles[x][y]) {
                    pos.x += mTileDimension.width;
                }
            }
        }

        // Top left
        node->pos[0].x = pos.x;
        node->pos[0].y = pos.y;

        // Top right
        node->pos[1].x = pos.x + (mTileDimension.width / 2);
        node->pos[1].y = pos.y;

        // Bottom left
        node->pos[2].x = pos.x;
        node->pos[2].y = pos.y + (mTileDimension.height / 2);

        // Bottom right
        node->pos[3].x = pos.x + (mTileDimension.width / 2);
        node->pos[3].y = pos.y + (mTileDimension.height / 2);
        break;
    }
    case TileTypeAuto: {
        // Top left
        if (x > 0 && y > 0) {
            if (tiles[x - 1][y - 1] != tiles[x][y] && tiles[x - 1][y] == tiles[x][y] && tiles[x][y - 1] == tiles[x][y]) {
                node->pos[0].x = mSize.x + mTileDimension.width;
                node->pos[0].y = mSize.y;
            } else if (tiles[x - 1][y] != tiles[x][y] && tiles[x][y - 1] != tiles[x][y]) {
                node->pos[0].x = mSize.x;
                node->pos[0].y = mSize.y + mTileDimension.height;
            } else if (tiles[x - 1][y] != tiles[x][y]) {
                node->pos[0].x = mSize.x;
                node->pos[0].y = mSize.y + (mTileDimension.height + (mTileDimension.height / 2));
            } else if (tiles[x][y - 1] != tiles[x][y]) {
                node->pos[0].x = mSize.x + (mTileDimension.width / 2);
                node->pos[0].y = mSize.y + mTileDimension.height;
            } else {
                node->pos[0].x = mSize.x + (mTileDimension.width / 2);
                node->pos[0].y = mSize.y + (mTileDimension.height + (mTileDimension.height / 2));
            }
        } else if (x > 0) {
            if (tiles[x - 1][y] != tiles[x][y]) {
                node->pos[0].x = mSize.x;
                node->pos[0].y = mSize.y + (mTileDimension.height + (mTileDimension.height / 2));
            } else {
                node->pos[0].x = mSize.x + (mTileDimension.width / 2);
                node->pos[0].y = mSize.y + (mTileDimension.height + (mTileDimension.height / 2));
            }
        } else if (y > 0) {
            if (tiles[x][y - 1] != tiles[x][y]) {
                node->pos[0].x = mSize.x + (mTileDimension.width / 2);
                node->pos[0].y = mSize.y + mTileDimension.height;
            } else {
                node->pos[0].x = mSize.x + (mTileDimension.width / 2);
                node->pos[0].y = mSize.y + (mTileDimension.height + (mTileDimension.height / 2));
            }
        } else {
            node->pos[0].x = mSize.x + (mTileDimension.width / 2);
            node->pos[0].y = mSize.y + (mTileDimension.height + (mTileDimension.height / 2));
        }

        // Top right
        if (x < width - 1 && y > 0) {
            if (tiles[x + 1][y - 1] != tiles[x][y] && tiles[x + 1][y] == tiles[x][y] && tiles[x][y - 1] == tiles[x][y]) {
                node->pos[1].x = mSize.x + (mTileDimension.width + (mTileDimension.width / 2));
                node->pos[1].y = mSize.y;
            } else if (tiles[x + 1][y] != tiles[x][y] && tiles[x][y - 1] != tiles[x][y]) {
                node->pos[1].x = mSize.x + (mTileDimension.width + (mTileDimension.width / 2));
                node->pos[1].y = mSize.y + mTileDimension.height;
            } else if (tiles[x + 1][y] != tiles[x][y]) {
                node->pos[1].x = mSize.x + (mTileDimension.width + (mTileDimension.width / 2));
                node->pos[1].y = mSize.y + (mTileDimension.height + (mTileDimension.height / 2));
            } else if (tiles[x][y - 1] != tiles[x][y]) {
                node->pos[1].x = mSize.x + mTileDimension.width;
                node->pos[1].y = mSize.y + mTileDimension.height;
            } else {
                node->pos[1].x = mSize.x + mTileDimension.width;
                node->pos[1].y = mSize.y + (mTileDimension.height + (mTileDimension.height / 2));
            }
        } else if (x < width - 1) {
            if (tiles[x + 1][y] != tiles[x][y]) {
                node->pos[1].x = mSize.x + (mTileDimension.width + (mTileDimension.width / 2));
                node->pos[1].y = mSize.y + (mTileDimension.height + (mTileDimension.height / 2));
            } else {
                node->pos[1].x = mSize.x + mTileDimension.width;
                node->pos[1].y = mSize.y + (mTileDimension.height + (mTileDimension.height / 2));
            }
        } else if (y > 0) {
            if (tiles[x][y - 1] != tiles[x][y]) {
                node->pos[1].x = mSize.x + mTileDimension.width;
                node->pos[1].y = mSize.y + mTileDimension.height;
            } else {
                node->pos[1].x = mSize.x + mTileDimension.width;
                node->pos[1].y = mSize.y + (mTileDimension.height + (mTileDimension.height / 2));
            }
        } else {
            node->pos[1].x = mSize.x + mTileDimension.width;
            node->pos[1].y = mSize.y + (mTileDimension.height + (mTileDimension.height / 2));
        }

        // Bottom left
        if (x > 0 && y < height - 1) {
            if (tiles[x - 1][y + 1] != tiles[x][y] && tiles[x - 1][y] == tiles[x][y] && tiles[x][y + 1] == tiles[x][y]) {
                node->pos[2].x = mSize.x + mTileDimension.width;
                node->pos[2].y = mSize.y + (mTileDimension.height / 2);
            } else if (tiles[x - 1][y] != tiles[x][y] && tiles[x][y + 1] != tiles[x][y]) {
                node->pos[2].x = mSize.x;
                node->pos[2].y = mSize.y + (mTileDimension.height * 2) + (mTileDimension.height / 2);
            } else if (tiles[x - 1][y] != tiles[x][y]) {
                node->pos[2].x = mSize.x;
                node->pos[2].y = mSize.y + (mTileDimension.height * 2);
            } else if (tiles[x][y + 1] != tiles[x][y]) {
                node->pos[2].x = mSize.x + (mTileDimension.width / 2);
                node->pos[2].y = mSize.y + (mTileDimension.height * 2) + (mTileDimension.height / 2);
            } else {
                node->pos[2].x = mSize.x + (mTileDimension.width / 2);
                node->pos[2].y = mSize.y + (mTileDimension.height + (mTileDimension.height / 2));
            }
        } else if (x > 0) {
            if (tiles[x - 1][y] != tiles[x][y]) {
                node->pos[2].x = mSize.x;
                node->pos[2].y = mSize.y + (mTileDimension.height * 2);
            } else {
                node->pos[2].x = mSize.x + (mTileDimension.width / 2);
                node->pos[2].y = mSize.y + (mTileDimension.height + (mTileDimension.height / 2));
            }
        } else if (y < height - 1) {
            if (tiles[x][y + 1] != tiles[x][y]) {
                node->pos[2].x = mSize.x + (mTileDimension.width / 2);
                node->pos[2].y = mSize.y + (mTileDimension.height * 2) + (mTileDimension.height / 2);
            } else {
                node->pos[2].x = mSize.x + (mTileDimension.width / 2);
                node->pos[2].y = mSize.y + (mTileDimension.height + (mTileDimension.height / 2));
            }
        } else {
            node->pos[2].x = mSize.x + (mTileDimension.width / 2);
            node->pos[2].y = mSize.y + (mTileDimension.height + (mTileDimension.height / 2));
        }

        // Bottom right
        if (x < width - 1 && y < height - 1) {
            if (tiles[x + 1][y + 1] != tiles[x][y] && tiles[x + 1][y] == tiles[x][y] && tiles[x][y + 1] == tiles[x][y]) {
                node->pos[3].x = mSize.x + (mTileDimension.width + (mTileDimension.width / 2));
                node->pos[3].y = mSize.y + (mTileDimension.height / 2);
            } else if (tiles[x + 1][y] != tiles[x][y] && tiles[x][y + 1] != tiles[x][y]) {
                node->pos[3].x = mSize.x + (mTileDimension.width + (mTileDimension.width / 2));
                node->pos[3].y = mSize.y + (mTileDimension.height * 2) + (mTileDimension.height / 2);
            } else if (tiles[x + 1][y] != tiles[x][y]) {
                node->pos[3].x = mSize.x + (mTileDimension.width + (mTileDimension.width / 2));
                node->pos[3].y = mSize.y + (mTileDimension.height * 2);
            } else if (tiles[x][y + 1] != tiles[x][y]) {
                node->pos[3].x = mSize.x + mTileDimension.width;
                node->pos[3].y = mSize.y + (mTileDimension.height * 2) + (mTileDimension.height / 2);
            } else {
                node->pos[3].x = mSize.x + mTileDimension.width;
                node->pos[3].y = mSize.y + (mTileDimension.height * 2);
            }
        } else if (x < width - 1) {
            if (tiles[x + 1][y] != tiles[x][y]) {
                node->pos[3].x = mSize.x + (mTileDimension.width + (mTileDimension.width / 2));
                node->pos[3].y = mSize.y + (mTileDimension.height * 2);
            } else {
                node->pos[3].x = mSize.x + mTileDimension.width;
                node->pos[3].y = mSize.y + (mTileDimension.height * 2);
            }
        } else if (y < height - 1) {
            if (tiles[x][y + 1] != tiles[x][y]) {
                node->pos[3].x = mSize.x + mTileDimension.width;
                node->pos[3].y = mSize.y + (mTileDimension.height * 2) + (mTileDimension.height / 2);
            } else {
                node->pos[3].x = mSize.x + mTileDimension.width;
                node->pos[3].y = mSize.y + (mTileDimension.height * 2);
            }
        } else {
            node->pos[3].x = mSize.x + mTileDimension.width;
            node->pos[3].y = mSize.y + (mTileDimension.height * 2);
        }

        break;
    }
    case TileTypeAutoHoriz: {
        // Top left
        if (x > 0 && tiles[x - 1][y] != tiles[x][y]) {
            node->pos[0].x = mSize.x;
            node->pos[0].y = mSize.y;
        } else {
            node->pos[0].x = mSize.x + (mTileDimension.width / 2);
            node->pos[0].y = mSize.y;
        }

        // Top right
        if (x < width - 1 && tiles[x + 1][y] != tiles[x][y]) {
            node->pos[1].x = mSize.x + (mTileDimension.width + (mTileDimension.width / 2));
            node->pos[1].y = mSize.y;
        } else {
            node->pos[1].x = mSize.x + mTileDimension.width;
            node->pos[1].y = mSize.y;
        }

        // Bottom left
        if (x > 0 && tiles[x - 1][y] != tiles[x][y]) {
            node->pos[2].x = mSize.x;
            node->pos[2].y = mSize.y + (mTileDimension.height / 2);
        } else {
            node->pos[2].x = mSize.x + (mTileDimension.width / 2);
            node->pos[2].y = mSize.y + (mTileDimension.height / 2);
        }

        // Bottom right
        if (x < width - 1 && tiles[x + 1][y] != tiles[x][y]) {
            node->pos[3].x = mSize.x + (mTileDimension.width + (mTileDimension.width / 2));
            node->pos[3].y = mSize.y + (mTileDimension.height / 2);
        } else {
            node->pos[3].x = mSize.x + mTileDimension.width;
            node->pos[3].y = mSize.y + (mTileDimension.height / 2);
        }

        break;
    }
    }

    return node;
}

Tileset::Tileset(const std::string& name)
    : mValid(false)
    , mTileDimension({ 32, 32 })
    , mImage(nullptr)
{
    std::shared_ptr<Json::Value> docPtr = AssetManager::get()->data(AssetManager::get()->Tileset, name);
    Json::Value doc = *docPtr;
    if (!doc.isObject()) {
        Logger::critical() << "Could not open tileset JSON file" << name;
        return;
    }

    if (!doc.isMember("image") || !doc.isMember("nodes") || !doc.isMember("collision")) {
        Logger::critical() << "Could not find required tileset JSON data for" << name;
        return;
    }

    mImage = AssetManager::get()->image(AssetManager::get()->Tileset, doc["image"].asString());
    if (!*mImage) {
        Logger::critical() << "Could not load tileset image for" << name;
        return;
    }

    mCollisionMap = doc["collision"].asString();

    if (doc.isMember("dimen")) {
        Json::Value dimenArr = doc["dimen"];
        if (dimenArr.size() == 2) {
            mTileDimension.width = dimenArr[0].asInt();
            mTileDimension.height = dimenArr[1].asInt();
        }
    }

    Json::Value nodes = doc["nodes"];

    uint32_t x = 0, y = 0;
    uint32_t index = 0;
    while (y < mImage->height()) {
        bool canAdd = true;
        for (const auto& i: mTypes) {
            if (i.second->contains({x, y})) {
                canAdd = false;
                break;
            }
        }

        if (canAdd && index < nodes.size()) {
            Json::Value nodeObj = nodes[index];
            if (!nodeObj.isMember("tile")) {
                Logger::critical() << "Could not find required tileset node JSON data for" << name;
                return;
            }

            if (y >= mImage->height()) {
                Logger::critical() << "Too many nodes in tileset data for" << name;
                return;
            }

            int frames = 0;
            if (nodeObj.isMember("animation")) {
                Json::Value animObj = nodeObj["animation"];
                if (animObj.isMember("frames")) {
                    frames = animObj["frames"].asInt();
                }
            }

            Types::Cells count(1, 1);
            if (nodeObj.isMember("count")) {
                Json::Value countObj = nodeObj["count"];
                if (countObj.size() == 2) {
                    count.cols = countObj[0].asInt();
                    count.rows = countObj[1].asInt();
                }
            }

            uint32_t base = 0;
            if (nodeObj.isMember("base")) {
                base = nodeObj["base"].asInt();
            }

            std::bitset<TilesetAttribute::Last> attrs;
            if (nodeObj.isMember("attributes")) {
                Json::Value attrsObj = nodeObj["attributes"];
                for (const auto& attrObj : attrsObj) {
                    std::string key = attrObj.asString();
                    if (key == "water") {
                        attrs[TilesetAttribute::Water] = true;
                    } else if (key == "above_row") {
                        attrs[TilesetAttribute::AboveRow] = true;
                    } else if (key == "above_all") {
                        attrs[TilesetAttribute::AboveAll] = true;
                    } else if (key == "toggle") {
                        attrs[TilesetAttribute::Toggle] = true;
                    } else if (key == "light_source") {
                        attrs[TilesetAttribute::LightSource] = true;
                    } else {
                        Logger::warning() << "Unknown attribute" << key << "for tileset" << name;
                    }
                }
            }

            std::shared_ptr<TilesetType> type = std::make_shared<TilesetType>(index, mTileDimension, nodeObj["tile"].asString(), attrs, x, y, frames, count, base);
            if (!*type) {
                return;
            }

            if (attrs[TilesetAttribute::LightSource] && nodeObj.isMember("light")) {
                Json::Value lightObj = nodeObj["light"];
                if (lightObj.isMember("base") && lightObj["base"].size() == 2) {
                    type->setLightBase(Types::Point<>(lightObj["base"][0].asInt(), lightObj["base"][1].asInt()));
                }

                if (lightObj.isMember("radius")) {
                    type->setLightRadius(lightObj["radius"].asInt());
                }

                if (lightObj.isMember("strength")) {
                    type->setLightStrength(lightObj["strength"].asInt() / 100.0f);
                }
            }

            mTypes[index++] = type;
        }

        x += mTileDimension.width;
        if (x >= mImage->width()) {
            x = 0;
            y += mTileDimension.height;
        }
    }

    if (mTypes.size() != nodes.size()) {
        Logger::warning() << "Possible missing nodes in tileset data for" << name;
    }

    mValid = true;
}

void Tileset::draw(Renderer& renderer, TilesetNode& node, const Types::Point<>& pos)
{
    Types::Rect<> dst(0, 0, mTileDimension.width / 2, mTileDimension.height / 2);
    int dx = 0, dy = 0;
    for (auto& po : node.pos) {
        dst.x = (pos.x * mTileDimension.width) + (dx * (mTileDimension.width / 2));
        dst.y = (pos.y * mTileDimension.height) + (dy * (mTileDimension.height / 2));

        Types::Rect<> src(po.x + (node.anim.x * std::floor(node.current)), po.y + (node.anim.y * std::floor(node.current)), dst.width, dst.height);
        if (node.toggled) {
            src.x += node.toggleWidth;
        }

        renderer.drawImage(*mImage, dst, src);

        dx++;
        if (dx > 1) {
            dx = 0;
            dy++;
        }
    }
}

std::shared_ptr<CollisionMap> Tileset::loadCollisionMap()
{
    std::shared_ptr<CollisionMap> collisionMap = AssetManager::get()->collision(AssetManager::get()->Tileset, mCollisionMap);
    if (!*collisionMap) {
        Logger::critical() << "Could not load tileset collision map" << mCollisionMap;
    }

    return collisionMap;
}

void Tileset::updateCollisionMap(CollisionMap& tilesetMap, CollisionMap& map, TilesetNode& node, uint32_t x, uint32_t y)
{
    Types::Dimension<> dimen(mTileDimension.width / 2, mTileDimension.height / 2);

    uint32_t dx = 0, dy = 0;
    for (auto& po : node.pos) {
        for (int cy = 0; cy < dimen.height; cy++) {
            for (int cx = 0; cx < dimen.width; cx++) {
                if (tilesetMap.get(po.x + cx, po.y + cy)) {
                    uint32_t dstx = (x * mTileDimension.width) + (dx * (mTileDimension.width / 2)) + cx;
                    uint32_t dsty = (y * mTileDimension.height) + (dy * (mTileDimension.height / 2)) + cy;
                    map.set(dstx, dsty, true);
                }
            }
        }

        dx++;
        if (dx > 1) {
            dx = 0;
            dy++;
        }
    }
}

bool Tileset::updateTiles(Types::Map2D& tiles, std::map<int, std::map<int, std::shared_ptr<TilesetNode>>>& map, uint32_t width, uint32_t height, TilesetAttribute::Type type)
{
    for (uint32_t x = 0; x < width; x++) {
        for (uint32_t y = 0; y < height; y++) {
            int n = tiles[x][y];
            if (n > 0) {
                if (static_cast<uint32_t>(n - 1) >= mTypes.size()) {
                    Logger::critical() << "Found an out of bounds node" << (n - 1) << ">" << mTypes.size();
                    return false;
                }

                auto tileType = mTypes[n - 1];
                if (tileType->hasAttribute(type) || tileType->checkBase(tiles, type, x, y)) {
                    map[y][x] = tileType->toNode(tiles, x, y, width, height);
                }
            }
        }
    }

    return true;
}
