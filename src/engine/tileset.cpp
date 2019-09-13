#include <cmath>

#include <json/value.h>

#include <engine/assetmanager.h>
#include <engine/collisionmap.h>
#include <engine/context.h>
#include <engine/graphics/bufferwriter.h>
#include <engine/graphics/image.h>
#include <engine/graphics/matrix.h>
#include <engine/graphics/renderer.h>
#include <engine/graphics/vector.h>
#include <engine/logger.h>
#include <engine/tileset.h>

using namespace engine;

constexpr TilesetAbove::Type TilesetAbove::Types[];

TilesetType::TilesetType(uint32_t index, Types::Dimension<>& tileDimension, const std::string& tileType, uint32_t x, uint32_t y)
    : mValid(false)
    , mIndex(index)
    , mTileDimension(tileDimension)
    , mTileType(TileTypeSingle)
    , mTileAbove(TilesetAbove::None)
    , mSize(x, y, tileDimension.width, tileDimension.height)
    , mFrames(0)
    , mCount({1, 1})
    , mBase(0)
    , mLightBase(-1, -1)
    , mLightRadius(0)
    , mLightColor({1.0f, 1.0f, 1.0f})
{
    switch (Types::hash(tileType.c_str())) {
    case Types::hash("automatic"):
        mTileType = TileTypeAuto;
        break;
    case Types::hash("automatic_horizontal"):
        mTileType = TileTypeAutoHoriz;
        break;
    case Types::hash("single"):
        mTileType = TileTypeSingle;
        break;
    default:
        Logger::critical("Tileset") << "Invalid tileset node tile" << tileType;
        return;
    }

    switch (mTileType) {
    case TileTypeAuto:
        mSize.width = tileDimension.width * 2;
        mSize.height = tileDimension.height * 3;
        break;
    case TileTypeAutoHoriz:
        mSize.width *= 2;
        break;
    default:
        break;
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

TilesetAbove::Type TilesetType::above() const
{
    return mTileAbove;
}

uint32_t TilesetType::index() const
{
    return mIndex;
}

Types::Point<> TilesetType::lightBase() const
{
    return mLightBase;
}

int32_t TilesetType::lightRadius() const
{
    return mLightRadius;
}

graphics::ColorGradient TilesetType::lightColor() const
{
    return mLightColor;
}

bool TilesetType::operator!() const
{
    return !mValid;
}

bool TilesetType::checkBase(Types::Map2D& tiles, TilesetAbove::Type above, uint32_t x, uint32_t y)
{
    if (mBase > 0) {
        switch (above) {
        case TilesetAbove::Row:
            return (y >= mBase && tiles[x][y - mBase] == tiles[x][y]);
        case TilesetAbove::All:
            return (y < mBase || tiles[x][y - mBase] != tiles[x][y]);
        default:
            break;
        }
    }

    return false;
}

void TilesetType::setAbove(TilesetAbove::Type above)
{
    mTileAbove = above;
}

void TilesetType::setAttributes(const std::bitset<TilesetAttribute::Last>& attrs)
{
    // Water tiles need to be drawn separately
    if (attrs[TilesetAttribute::Water]) {
        mTileAbove = TilesetAbove::Water;
    }

    mAttributes = attrs;
    if (mAttributes[TilesetAttribute::Toggle]) {
        mSize.width *= 2;
    }
}

void TilesetType::setBase(uint32_t base)
{
    mBase = base;
}

void TilesetType::setCount(Types::Cells count)
{
    mCount = count;
    if (mTileType == TileTypeSingle) {
        mSize.width *= mCount.cols;
        mSize.height *= mCount.rows;
    }
}

void TilesetType::setFrames(int8_t frames)
{
    mFrames = frames;
    if (mFrames > 0) {
        switch (mTileType) {
        case TileTypeAutoHoriz:
            mSize.height *= mFrames;
            break;
        default:
            mSize.width *= mFrames;
            break;
        }
    }
}

void TilesetType::setLightBase(Types::Point<> base)
{
    mLightBase = base;
}

void TilesetType::setLightRadius(int32_t radius)
{
    mLightRadius = radius;
}

void TilesetType::setLightColor(const graphics::ColorGradient& color)
{
    mLightColor = color;
}


std::shared_ptr<TilesetNode> TilesetType::toNode(Types::Map2D& tiles, uint32_t x, uint32_t y, uint32_t width, uint32_t height)
{
    std::shared_ptr<TilesetNode> node = std::make_shared<TilesetNode>();
    node->id = x + (y * width);
    node->animSize = Types::Point<>(mTileDimension.width, 0);
    node->frames = mFrames;
    node->toggleWidth = (mAttributes[TilesetAttribute::Toggle]) ? (mSize.width / 2) : 0;
    node->toggled = false;
    node->type = this;

    if (mTileType == TileTypeAuto) {
        node->animSize.x *= 2;
    } else if (mTileType == TileTypeAutoHoriz) {
        node->animSize.x = 0;
        node->animSize.y = mTileDimension.height;
    }

    if (node->frames <= 1) {
        node->animSize = Types::Point<>(0, 0);
    }

    uint32_t chunkWidth = (mTileDimension.width / 2);
    uint32_t chunkHeight = (mTileDimension.height / 2);

    switch (mTileType) {
    case TileTypeSingle: {
        // The id of the tile is the top-left corner of the tile block
        uint32_t startX = x, startY = y;

        Types::Point<uint32_t> pos(mSize.x, mSize.y);
        for(uint32_t iy = 1; iy < mCount.rows; iy++) {
            if (y - iy >= 0) {
                if (tiles[x][y - iy] == tiles[x][y]) {
                    pos.y += mTileDimension.height;
                    startY = y - iy;
                }
            }
        }
        for(uint32_t ix = 1; ix < mCount.cols; ix++) {
            if (x - ix >= 0) {
                if (tiles[x - ix][y] == tiles[x][y]) {
                    pos.x += mTileDimension.width;
                    startX = x - ix;
                }
            }
        }

        node->id = std::min(node->id, startX + (startY * width));

        // Top left
        node->pos[0].x = pos.x;
        node->pos[0].y = pos.y;

        // Top right
        node->pos[1].x = pos.x + chunkWidth;
        node->pos[1].y = pos.y;

        // Bottom left
        node->pos[2].x = pos.x;
        node->pos[2].y = pos.y + chunkHeight;

        // Bottom right
        node->pos[3].x = pos.x + chunkWidth;
        node->pos[3].y = pos.y + chunkHeight;
        break;
    }
    case TileTypeAuto: {
        /*
        Horizontal auto tilesets are set up in 24x24 pixel chunks.
        XX XX 02 03
        XX XX 06 07
        08 09 10 11
        12 13 14 15
        16 17 18 19
        20 21 22 23
        XX - Unused
        02, 03, 06, 07 - Inner corners
        08, 11, 20, 23 - Outer corners
        */

        // Top left
        uint8_t tlX = 0, tlY = 0;
        if (x > 0 && y > 0) {
            if (tiles[x - 1][y - 1] != tiles[x][y] && tiles[x - 1][y] == tiles[x][y] && tiles[x][y - 1] == tiles[x][y]) {
                tlX = 2;
            } else if (tiles[x - 1][y] != tiles[x][y] && tiles[x][y - 1] != tiles[x][y]) {
                tlY = 2;
            }
        }
        if (tlX == 0 && tlY == 0) {
            if (x > 0 && tiles[x - 1][y] != tiles[x][y]) {
                tlY = 4;
            } else if (y > 0 && tiles[x][y - 1] != tiles[x][y]) {
                tlX = 2;
                tlY = 2;
            } else {
                tlX = 2;
                tlY = 4;
            }
        }

        // Top right
        uint8_t trX = 0, trY = 0;
        if (x < width - 1 && y > 0) {
            if (tiles[x + 1][y - 1] != tiles[x][y] && tiles[x + 1][y] == tiles[x][y] && tiles[x][y - 1] == tiles[x][y]) {
                trX = 3;
            } else if (tiles[x + 1][y] != tiles[x][y] && tiles[x][y - 1] != tiles[x][y]) {
                trX = 3;
                trY = 2;
            }
        }

        if (trX == 0 && trY == 0) {
            if (x < width - 1 && tiles[x + 1][y] != tiles[x][y]) {
                trX = 3;
                trY = 4;
            } else if (y > 0 && tiles[x][y - 1] != tiles[x][y]) {
                trX = 1;
                trY = 2;
            } else {
                trX = 1;
                trY = 4;
            }
        }

        // Bottom left
        uint8_t blX = 0, blY = 0;
        if (x > 0 && y < height - 1) {
            if (tiles[x - 1][y + 1] != tiles[x][y] && tiles[x - 1][y] == tiles[x][y] && tiles[x][y + 1] == tiles[x][y]) {
                blX = 2;
                blY = 1;
            } else if (tiles[x - 1][y] != tiles[x][y] && tiles[x][y + 1] != tiles[x][y]) {
                blY = 5;
            }
        }

        if (blX == 0 && blY == 0) {
            if (x > 0 && tiles[x - 1][y] != tiles[x][y]) {
                blY = 3;
            } else if (y < height - 1 && tiles[x][y + 1] != tiles[x][y]) {
                blX = 2;
                blY = 5;
            } else {
                blX = 2;
                blY = 3;
            }
        }

        // Bottom right
        uint8_t brX = 0, brY = 0;
        if (x < width - 1 && y < height - 1) {
            if (tiles[x + 1][y + 1] != tiles[x][y] && tiles[x + 1][y] == tiles[x][y] && tiles[x][y + 1] == tiles[x][y]) {
                brX = 3;
                brY = 1;
            } else if (tiles[x + 1][y] != tiles[x][y] && tiles[x][y + 1] != tiles[x][y]) {
                brX = 3;
                brY = 5;
            }
        }

        if (brX == 0 && brY == 0) {
            if (x < width - 1 && tiles[x + 1][y] != tiles[x][y]) {
                brX = 3;
                brY = 3;
            } else if (y < height - 1 && tiles[x][y + 1] != tiles[x][y]) {
                brX = 1;
                brY = 5;
            } else {
                brX = 1;
                brY = 3;
            }
        }

        node->pos[0].x = mSize.x + chunkWidth * tlX;
        node->pos[0].y = mSize.y + chunkHeight * tlY;

        node->pos[1].x = mSize.x + chunkWidth * trX;
        node->pos[1].y = mSize.y + chunkHeight * trY;

        node->pos[2].x = mSize.x + chunkWidth * blX;
        node->pos[2].y = mSize.y + chunkHeight * blY;

        node->pos[3].x = mSize.x + chunkWidth * brX;
        node->pos[3].y = mSize.y + chunkHeight * brY;

        break;
    }
    case TileTypeAutoHoriz: {
        // Top left
        uint8_t tlX = 0, tlY = 0;
        if (!(x > 0 && tiles[x - 1][y] != tiles[x][y])) {
            tlX = 2; 
        }

        // Top right
        uint8_t trX = 0, trY = 0;
        if (x < width - 1 && tiles[x + 1][y] != tiles[x][y]) {
            trX = 3;
        } else {
            trX = 1;
        }

        // Bottom left
        uint8_t blX = 0, blY = 0;
        if (x > 0 && tiles[x - 1][y] != tiles[x][y]) {
            blY = 1;
        } else {
            blX = 2;
            blY = 1;
        }

        // Bottom right
        uint8_t brX = 0, brY = 0;
        if (x < width - 1 && tiles[x + 1][y] != tiles[x][y]) {
            brX = 3;
            brY = 1;
        } else {
            brX = 1;
            brY = 1;
        }

        node->pos[0].x = mSize.x + tlX * chunkWidth;
        node->pos[0].y = mSize.y + tlY * chunkHeight;

        node->pos[1].x = mSize.x + trX * chunkWidth;
        node->pos[1].y = mSize.y + trY * chunkHeight;

        node->pos[2].x = mSize.x + blX * chunkWidth;
        node->pos[2].y = mSize.y + blY * chunkHeight;

        node->pos[3].x = mSize.x + brX * chunkWidth;
        node->pos[3].y = mSize.y + brY * chunkHeight;

        break;
    }
    }

    return node;
}

Tileset::Tileset(std::shared_ptr<Context>& ctx, const std::string& name)
    : ContextObject(ctx)
    , mValid(false)
    , mTileDimension({ 32, 32 })
    , mImage(nullptr)
{
    std::unique_ptr<Json::Value> docPtr = context().assetManager().data(AssetManager::Tileset, name);
    Json::Value doc = *docPtr;
    if (!doc.isObject()) {
        Logger::critical("Tileset") << "Could not open tileset JSON file" << name;
        return;
    }

    if (!doc.isMember("image") || !doc.isMember("nodes") || !doc.isMember("collision")) {
        Logger::critical("Tileset") << "Could not find required tileset JSON data for" << name;
        return;
    }

    mImage = context().assetManager().image(AssetManager::Tileset, doc["image"].asString());
    if (!mImage) {
        Logger::critical("Tileset") << "Could not load tileset image for" << name;
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
            if (!nodeObj.isMember("index") || !nodeObj.isMember("tile")) {
                Logger::critical("Tileset") << "Could not find required tileset node JSON data for" << name;
                return;
            }

            if (y >= mImage->height()) {
                Logger::critical("Tileset") << "Too many nodes in tileset data for" << name;
                return;
            }

            uint32_t nodeIndex = nodeObj["index"].asInt();
            if (mTypes.find(nodeIndex) != mTypes.end()) {
                Logger::critical("Tileset") << "Duplicate index" << nodeIndex << "for" << name;
                return;
            }

            std::shared_ptr<TilesetType> type = std::make_shared<TilesetType>(nodeIndex, mTileDimension, nodeObj["tile"].asString(), x, y);
            if (!*type) {
                return;
            }

            if (nodeObj.isMember("above")) {
                switch (Types::hash(nodeObj["above"].asString().c_str())) {
                case Types::hash("row"):
                    type->setAbove(TilesetAbove::Row);
                    break;
                case Types::hash("all"):
                    type->setAbove(TilesetAbove::All);
                    break;
                default:
                    Logger::warning("Tileset") << "Unknown above" << nodeObj["above"].asString() << "for tileset" << name;
                    break;
                }
            }

            if (nodeObj.isMember("attributes")) {
                Json::Value attrsObj = nodeObj["attributes"];

                std::bitset<TilesetAttribute::Last> attrs;
                for (const auto& attrObj : attrsObj) {
                    std::string key = attrObj.asString();
                    switch (Types::hash(key.c_str())) {
                    case Types::hash("water"):
                        attrs[TilesetAttribute::Water] = true;
                        break;
                    case Types::hash("toggle"):
                        attrs[TilesetAttribute::Toggle] = true;
                        break;
                    case Types::hash("light_source"):
                        attrs[TilesetAttribute::LightSource] = true;
                        break;
                    case Types::hash("path"):
                        attrs[TilesetAttribute::Path] = true;
                        break;
                    default:
                        Logger::warning("Tileset") << "Unknown attribute" << key << "for tileset" << name;
                        break;
                    }
                }

                type->setAttributes(attrs);
            }

            if (nodeObj.isMember("animation")) {
                Json::Value animObj = nodeObj["animation"];
                if (animObj.isMember("frames")) {
                    type->setFrames(static_cast<uint8_t>(animObj["frames"].asInt()));
                }
            }

            if (nodeObj.isMember("base")) {
                type->setBase(nodeObj["base"].asInt());
            }

            if (nodeObj.isMember("count")) {
                Json::Value countObj = nodeObj["count"];
                if (countObj.size() == 2) {
                    Types::Cells count(1, 1);
                    count.cols = countObj[0].asInt();
                    count.rows = countObj[1].asInt();
                    type->setCount(count);
                }
            }

            if (type->hasAttribute(TilesetAttribute::LightSource) && nodeObj.isMember("light")) {
                Json::Value lightObj = nodeObj["light"];
                if (lightObj.isMember("base") && lightObj["base"].size() == 2) {
                    type->setLightBase(Types::Point<>(lightObj["base"][0].asInt(), lightObj["base"][1].asInt()));
                }

                if (lightObj.isMember("radius")) {
                    type->setLightRadius(lightObj["radius"].asInt());
                }

                float_t strength = 1.0f;
                if (lightObj.isMember("strength")) {
                    strength = lightObj["strength"].asInt() / 255.0f;
                }

                if (lightObj.isMember("color") && lightObj["color"].size() >= 3) {
                    Json::Value colorObj = lightObj["color"];
                    auto g = graphics::ColorGradient(graphics::Color::fromInt(colorObj[0].asInt(), colorObj[1].asInt(), colorObj[2].asInt()));
                    g *= strength;
                    type->setLightColor(g);
                }
            }

            mTypes[nodeIndex] = type;
            index++;
        }

        x += mTileDimension.width;
        if (x >= mImage->width()) {
            x = 0;
            y += mTileDimension.height;
        }
    }

    if (mTypes.size() != nodes.size()) {
        Logger::warning("Tileset") << "Possible missing nodes in tileset data for" << name;
    }

    mValid = true;
}

graphics::Image& Tileset::image() const
{
    return *mImage;
}

void Tileset::updateBuffer(graphics::Renderer& renderer, TilesetNode& node, const Types::Point<>& pos, graphics::BufferWriter& writer, uint32_t texture, float_t zOrder)
{
    Types::Rect<> dst(0, 0, mTileDimension.width / 2, mTileDimension.height / 2);
    int32_t dx = 0, dy = 0;
    for (auto& po : node.pos) {
        dst.x = (pos.x * mTileDimension.width) + (dx * (mTileDimension.width / 2));
        dst.y = (pos.y * mTileDimension.height) + (dy * (mTileDimension.height / 2));

        Types::Rect<> src(po.x, po.y, dst.width, dst.height);
        if (node.toggled) {
            src.x += node.toggleWidth;
        }

        writer += engine::graphics::Vector3D(dst.x, dst.y, zOrder);
        writer += engine::graphics::Vector4D(src.x, src.y, mTileDimension.width / 2.0f, mTileDimension.height / 2.0f);
        writer += engine::graphics::Vector2D(node.animSize.x, node.animSize.y);
        writer += static_cast<float_t>(texture);

        dx++;
        if (dx > 1) {
            dx = 0;
            dy++;
        }
    }
}

std::unique_ptr<CollisionMap> Tileset::loadCollisionMap()
{
    std::unique_ptr<CollisionMap> collisionMap = context().assetManager().collision(AssetManager::Tileset, mCollisionMap);
    if (!collisionMap) {
        Logger::critical("Tileset") << "Could not load tileset collision map" << mCollisionMap;
    }

    return collisionMap;
}

void Tileset::updateCollisionMap(const CollisionMap& tilesetMap, CollisionMap& outMap, const TilesetNode& node, uint32_t x, uint32_t y)
{
    Types::Dimension<> dimen(mTileDimension.width / 2, mTileDimension.height / 2);

    uint32_t dx = 0, dy = 0;
    for (const auto po : node.pos) {
        for (int32_t cy = 0; cy < dimen.height; cy++) {
            for (int32_t cx = 0; cx < dimen.width; cx++) {
                uint32_t dstx = (x * mTileDimension.width) + (dx * (mTileDimension.width / 2)) + cx;
                uint32_t dsty = (y * mTileDimension.height) + (dy * (mTileDimension.height / 2)) + cy;
                outMap.set(dstx, dsty, tilesetMap.get(po.x + cx, po.y + cy));
            }
        }

        dx++;
        if (dx > 1) {
            dx = 0;
            dy++;
        }
    }
}

bool Tileset::updateTiles(Types::Map2D& tiles, std::map<int32_t, std::map<int32_t, std::shared_ptr<TilesetNode>>>& map, uint32_t width, uint32_t height, TilesetAbove::Type above)
{
    for (uint32_t x = 0; x < width; x++) {
        for (uint32_t y = 0; y < height; y++) {
            int32_t n = tiles[x][y];
            if (n > 0) {
                auto it = mTypes.find(n);
                if (it == mTypes.end()) {
                    Logger::critical("Tileset") << "Found an out of bounds tile node" << (n - 1);
                    return false;
                }

                auto tileType = (*it).second;
                if (tileType->above() == above || tileType->checkBase(tiles, above, x, y)) {
                    map[y][x] = tileType->toNode(tiles, x, y, width, height);
                }
            }
        }
    }

    return true;
}
