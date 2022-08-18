#include <algorithm>
#include <cmath>

#include <json/value.h>

#include <engine/assetmanager.h>
#include <engine/collisionmap.h>
#include <engine/context.h>
#include <engine/graphics/matrix.h>
#include <engine/graphics/renderer.h>
#include <engine/graphics/vector.h>
#include <engine/logger.h>
#include <engine/map.h>
#include <engine/maplayer.h>

using namespace engine;

Map::Map(std::shared_ptr<Context>& ctx, graphics::Renderer& renderer, const std::string& id)
    : ContextObject(ctx)
    , mValid(false)
    , mID(id)
    , mType(Map::Type::Outside)
    , mDimensions(0, 0)
    , mNeedUpdate(true)
    , mAnimFrame(0)
{
    Logger::debug("Map") << "Loading Map" << id;

    auto doc = context().assetManager().data(AssetManager::Map, id);
    if (!doc || !doc->isObject()) {
        Logger::critical("Map") << "Invalid JSON data for" << id;
        return;
    }

    if (!doc->isMember("width") || !doc->isMember("height") || !doc->isMember("layers")) {
        Logger::critical("Map") << "Could not find required map JSON attributes for" << id;
        return;
    }

    mDimensions.width = (*doc)["width"].asInt();
    mDimensions.height = (*doc)["height"].asInt();

    if (doc->isMember("type")) {
        auto type = (*doc)["type"].asString();
        switch (Types::hash(type.c_str())) {
        case Types::hash("inside"):
            mType = Map::Type::Inside;
            break;
        default:
            break;
        }
    }

    uint32_t tilesetIndex = 0;

    Json::Value layers = (*doc)["layers"];
    for (auto layerObj : layers) {
        if (!layerObj.isMember("tileset")) {
            Logger::critical("Map") << "Could not find required map layer JSON attributes for" << id;
            return;
        }

        std::string name = layerObj["tileset"].asString();
        if (mTilesets.find(name) != mTilesets.end()) {
            continue;
        }

        mTilesets[name] = std::make_shared<Tileset>(contextPtr(), name);
        if (!*mTilesets[name]) {
            Logger::critical("Map") << "Could not load tileset for" << id;
            return;
        }

        mTilesetIndexes[name] = tilesetIndex++;
    }

    for (auto layerObj : layers) {
        std::string name = layerObj["tileset"].asString();
        if (mTilesets.find(name) == mTilesets.end()) {
            Logger::critical("Map") << "Could not find tileset for" << name;
            return;
        }

        if (!layerObj.isMember("data")) {
            Logger::critical("Map") << "Could not find required layer JSON attributes for" << id;
            return;
        }

        std::unordered_map<int32_t, std::unordered_map<int32_t, int32_t>> data;
        Json::Value dataObj = layerObj["data"];
        for (uint32_t x = 0; x < mDimensions.width; x++) {
            for (uint32_t y = 0; y < mDimensions.height; y++) {
                data[x][y] = dataObj[x + (y * mDimensions.width)].asInt();
            }
        }

        std::shared_ptr<MapLayer> layer = std::make_shared<MapLayer>(renderer, data, mTilesets.find(name)->second, mDimensions.width, mDimensions.height, mTilesetIndexes[name]);
        if (!*layer) {
            Logger::critical("Map") << "Could not load layer for" << id;
            return;
        }

        mLayers.push_back(layer);
    }

    mCollisionMap = std::make_unique<CollisionMap>(pixelWidth(), pixelHeight());
    for (const auto& layer : mLayers) {
        if (!layer->updateCollisionMap(*mCollisionMap)) {
            Logger::warning("Map") << "Could not load collision map for" << id;
        }

        if (!layer->updateLightSources(mLights)) {
            Logger::warning("Map") << "Could not load light sources for" << id;
        }

        if (!layer->updatePaths(mPaths)) {
            Logger::warning("Map") << "Could not load paths for" << id;
        }
    }

    uint32_t bufferSize = engine::graphics::Vector4D::Size() * 2 + engine::graphics::Vector2D::Size() + sizeof(float_t) * 2;
    for (auto above: TilesetAbove::Types) {
        if (above == TilesetAbove::Row) {
            for (uint32_t r = 0; r < mDimensions.height; ++r) {
                mRowBufferCount[r] = tilesCount(above, r);
                mRowBuffer[r] = renderer.createBuffer(bufferSize * mRowBufferCount[r]);
            }
        } else {
            mBufferCount[above] = tilesCount(above);
            mBuffer[above] = renderer.createBuffer(bufferSize * mBufferCount[above]);
        }
    }

    uint32_t w = 0, h = 0;
    for (auto& t: mTilesets) {
        auto& i = t.second->image();
        w = std::max(w, i.width());
        h = std::max(h, i.height());
    }

    mTexture = renderer.createTexture(w, h, mTilesets.size());

    for (auto& t: mTilesets) {
        auto& img = t.second->image();
        mTexture->setData(img, mTilesetIndexes[t.first]);
    }

    mValid = true;
}

bool Map::animate(uint64_t frameDiff)
{
    float_t c = mAnimFrame;
    c += frameDiff / 200.0f;
    if (c >= 3) {
        c = 0;
    }

    bool changed = std::floor(c) != std::floor(mAnimFrame);
    mAnimFrame = c;
    return changed;
}

void Map::drawBuffer(graphics::Renderer& renderer, const Types::Point<>& dst, TilesetAbove::Type above)
{
    renderer.drawTexturesAnim(dst, mTexture.get(), mBuffer[above].get(), std::floor(mAnimFrame), mBufferCount[above]);
}

void Map::drawRowBuffer(graphics::Renderer& renderer, const Types::Point<>& dst, int32_t row)
{
    renderer.drawTexturesAnim(dst, mTexture.get(), mRowBuffer[row].get(), std::floor(mAnimFrame), mRowBufferCount[row]);
}

void Map::updateBuffers(graphics::Renderer& renderer)
{
    if (!mNeedUpdate) {
        return;
    }

    for (auto above: TilesetAbove::Types) {
        if (above == TilesetAbove::Row) {
            for (int32_t r = 0; r < mDimensions.height; ++r) {
                auto writer = mRowBuffer[r]->writer();
                for (const auto& layer : mLayers) {
                    layer->updateRowBuffer(renderer, above, r, writer);
                }
            }
        } else {
            auto writer = mBuffer[above]->writer();
            for (const auto& layer : mLayers) {
                layer->updateBuffer(renderer, above, writer);
            }
        }
    }

    mNeedUpdate = false;
}

void Map::checkCollision(const Types::Point<float_t>& pos, const Types::Dimension<>& size, Types::Point<float_t>& dst, Types::Point<float_t>& velocity) const
{
    if (pos.x + dst.x <= 0.0f) {
        velocity.x = 0.0f;
        dst.x = -pos.x;
    }

    if (pos.y + dst.y <= 0.0f) {
        velocity.y = 0.0f;
        dst.y = -pos.y;
    }

    if (velocity.x == 0.0f && velocity.y == 0.0f) {
        return;
    }

    // Nothing to be done if we haven't moved a whole pixel
    int32_t xdst = static_cast<int32_t>(pos.x + dst.x) - static_cast<int32_t>(pos.x);
    int32_t ydst = static_cast<int32_t>(pos.y + dst.y) - static_cast<int32_t>(pos.y);
    if (xdst == 0 && ydst == 0) {
        return;
    }

    // We may not have any velocity here if we tried to move out of bounds, so
    // it's unnecessary to check for collision.
    if (velocity.x == 0.0f) {
        xdst = 0;
    }
    if (velocity.y == 0.0f) {
        ydst = 0;
    }

    Types::Quad<> diff;
    Types::Pair diffPos = {0, 0};
    Types::Point<float_t> dstMod(0.0f, 0.0f);
    auto found = isColliding(Types::Point<int32_t>(pos.x, pos.y), {xdst, ydst}, size, diff, diffPos);
    if (found.first) {
        // diffPos == 0 means that we can't move around the object, so we reset the velocity
        if (diffPos.first == 0) {
            velocity.x = 0.0f;
            dst.x = 0.0f;
        } else if (!found.second && dst.y == 0.0f) {
            if (diffPos.first < 0) {
                dstMod.y = -std::min<float_t>(diff.top, std::abs(dst.x));
            } else {
                dstMod.y = std::min<float_t>(diff.bottom, std::abs(dst.x));
            }
        }

        // Use the position remainder to ensure we end up at a whole pixel position
        float_t remain = pos.x - std::floor(pos.x);
        if (diff.left >= 0) {
            dst.x = -(diff.left + remain);
        } else {
            dst.x = diff.right - remain;
        }
    }

    if (found.second) {
        // diffPos == 0 means that we can't move around the object, so we reset the velocity
        if (diffPos.second == 0) {
            velocity.y = 0.0f;
            dst.y = 0.0f;
        } else if (!found.first && dst.x == 0.0f) {
            if (diffPos.second < 0) {
                dstMod.x = -std::min<float_t>(diff.left, std::abs(dst.y));
            } else if (diffPos.second > 0) {
                dstMod.x = std::min<float_t>(diff.right, std::abs(dst.y));
            }
        }

        float_t remain = pos.y - std::floor(pos.y);
        if (diff.top >= 0) {
            dst.y = -(diff.top + remain);
        } else {
            dst.y = diff.bottom - remain;
        }
    }

    // Don't try to move around if we would end up colliding
    if (dstMod.x != 0.0f || dstMod.y != 0.0f) {
        dst.x = (dstMod.x != 0.0f) ? dstMod.x : dst.x;
        dst.y = (dstMod.y != 0.0f) ? dstMod.y : dst.y;
        xdst = static_cast<int32_t>(pos.x + dst.x) - static_cast<int32_t>(pos.x);
        ydst = static_cast<int32_t>(pos.y + dst.y) - static_cast<int32_t>(pos.y);
        auto f = mCollisionMap->check(Types::Point<int32_t>(pos.x, pos.y), {xdst, ydst}, size);
        if (f.first) {
            dst.x = 0.0f;
        }
        if (f.second) {
            dst.y = 0.0f;
        }
    }
}

bool Map::isNodeSolid(int32_t x, int32_t y, const Types::Dimension<>& size) const
{
    Types::Dimension<> d = getTileDimension();
    return mCollisionMap->solid(Types::Point<uint32_t>(x * d.width, y * d.height), {((size.width / d.width) + 1) * d.width, ((size.height / d.height) + 1) * d.height});
}

bool Map::isNodePath(int32_t x, int32_t y) const
{
    return std::find(mPaths.begin(), mPaths.end(), Types::Point<int32_t>(x, y)) != mPaths.end();
}

void Map::addLightSources(std::vector<std::shared_ptr<Overlay::LightSource>>& sources)
{
    for (auto& s: mLights) {
        sources.push_back(s);
    }
}

void Map::toggleLights(bool on)
{
    for (auto& layer : mLayers) {
        if (layer->toggleLights(on)) {
            mNeedUpdate = true;
        }
    }
}

std::pair<bool, bool> Map::isColliding(const Types::Point<int32_t>& pos, const Types::Point<>& dst, const Types::Dimension<>& size, Types::Quad<>& diff, Types::Pair& diffPos) const
{
    diffPos = {0, 0};

    auto found = mCollisionMap->check(pos, dst, size, &diff);

    // If we have a collision, see if we can move around it.
    // Prefer top and left if the differences are equal
    if (found.first) {
        int32_t obsdiff = std::max<uint32_t>(1, size.height / 5);
        if (diff.top >= obsdiff && (diff.bottom <= 0 || diff.top >= diff.bottom)) {
            diffPos.first = -1;
        } else if (diff.bottom >= obsdiff && (diff.top <= 0 || diff.bottom > diff.top)) {
            diffPos.first = 1;
        }
    }

    // Found collision on Y axis, check if we can move around
    if (found.second) {
        int32_t obsdiff = std::max<uint32_t>(1, size.width / 5);
        if (diff.left >= obsdiff && (diff.right <= 0 || diff.left >= diff.right)) {
            diffPos.second = -1;
        } else if (diff.right >= obsdiff && (diff.left <= 0 || diff.right > diff.left)) {
            diffPos.second = 1;
        }
    }

    return found;
}

uint32_t Map::tilesCount(TilesetAbove::Type above, int32_t row)
{
    uint32_t ret = 0;
    for (const auto& layer : mLayers) {
        ret += layer->tilesCount(above, row);
    }

    return ret;
}

const std::string& Map::id() const
{
    return mID;
}

Map::Type::Val Map::type() const
{
    return mType;
}

uint32_t Map::width() const
{
    return mDimensions.width;
}

uint32_t Map::pixelWidth() const
{
    return mDimensions.width * getTileDimension().width;
}

uint32_t Map::height() const
{
    return mDimensions.height;
}

uint32_t Map::pixelHeight() const
{
    return mDimensions.height * getTileDimension().height;
}

Types::Dimension<> Map::getTileDimension() const
{
    for (auto tileset : mTilesets) {
        return tileset.second->getTileDimension();
    }

    return { 32, 32 };
}

bool Map::operator!() const
{
    return !mValid;
}

MapLightSource::MapLightSource(Types::Point<int32_t> pos, int32_t radius, const graphics::ColorGradient& color)
    : mPosition(pos)
    , mRadius(radius)
    , mColor(color)
{
}

Types::Point<int32_t> MapLightSource::lightPosition()
{
    return mPosition;
}

int32_t MapLightSource::lightRadius()
{
    return mRadius;
}

graphics::ColorGradient MapLightSource::lightColor()
{
    return mColor;
}
