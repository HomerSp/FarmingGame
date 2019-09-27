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
    , mBufferCount(0)
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
        mBufferCount[above] = tilesCount(above);
        mBuffer[above] = renderer.createBuffer(bufferSize * mBufferCount[above]);
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

void Map::updateBuffers(graphics::Renderer& renderer)
{
    if (!mNeedUpdate) {
        return;
    }

    for (auto above: TilesetAbove::Types) {
        auto writer = mBuffer[above]->writer();
        for (const auto& layer : mLayers) {
            layer->updateBuffer(renderer, above, writer);
        }

        writer.release();
    }

    mNeedUpdate = false;
}

void Map::checkCollision(const Types::Point<float_t>& pos, const Types::Dimension<>& size, Types::Point<float_t>& dst, float_t& velocityX, float_t& velocityY) const
{
    if (pos.x + dst.x < 0.0f) {
        dst.x = 0.0f;
        velocityX = 0.0f;
    }

    if (pos.y + dst.y < 0.0f) {
        dst.y = 0.0f;
        velocityY = 0.0f;
    }

    // No need to continue checking.
    if (dst.x == 0.0f && dst.y == 0.0f) {
        return;
    }

    Types::Pair diff;
    int8_t rDiff = 0;
    if (dst.x != 0.0f && isColliding({ pos.x + dst.x, pos.y }, size, diff, rDiff, false)) {
        // rDiff == 0 means that we can't move around the object, so we reset the velocity
        if (rDiff == 0) {
            velocityX = 0.0f;
        } else if (dst.y == 0.0f) {
            if (rDiff < 0) {
                dst.y = (dst.x < 0.0f) ? dst.x : -dst.x;
            } else if (rDiff > 0) {
                dst.y = (dst.x < 0.0f) ? -dst.x : dst.x;
            }
        }

        if (dst.x < 0.0f) {
            dst.x = diff.first;
        } else {
            dst.x = diff.second;
        }
    }

    if (dst.y != 0.0f && isColliding({ pos.x, pos.y + dst.y }, size, diff, rDiff, true)) {
        // rDiff == 0 means that we can't move around the object, so we reset the velocity
        if (rDiff == 0) {
            velocityY = 0.0f;
        } else if (dst.x == 0.0f) {
            if (rDiff < 0) {
                dst.x = (dst.y < 0.0f) ? dst.y : -dst.y;
            } else if (rDiff > 0) {
                dst.x = (dst.y < 0.0f) ? -dst.y : dst.y;
            }

            // We need to check x collision again to make sure we don't get stuck
            Types::Pair diffx;
            if (dst.x != 0.0f && isColliding({ pos.x + dst.x, pos.y }, size, diffx, rDiff, false)) {
                if (rDiff == 0) {
                    velocityX = 0.0f;
                }

                if (dst.x < 0.0f) {
                    dst.x = diffx.first;
                } else {
                    dst.x = diffx.second;
                }
            }
        }

        if (dst.y < 0.0f) {
            dst.y = diff.first;
        } else {
            dst.y = diff.second;
        }
    }
}

bool Map::isNodeSolid(int32_t x, int32_t y, const Types::Dimension<>& size) const
{
    Types::Dimension<> d = getTileDimension();
    return mCollisionMap->get(x * d.width, y * d.height, ((size.width / d.width) + 1) * d.width, ((size.height / d.height) + 1) * d.height);
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

bool Map::isColliding(const Types::Point<float_t>& pos, const Types::Dimension<>& size, Types::Pair& diff, int8_t& rDiff, bool vertical) const
{
    rDiff = 0;

    Types::Quad<> foundDiff;
    bool found = mCollisionMap->get(pos.x, pos.y, size.width, size.height, &foundDiff);

    // Check if we can move around the obstacle.
    if (vertical) {
        diff.first = foundDiff.y1;
        diff.second = foundDiff.y2;

        int32_t obsdiff = size.width / 2;
        if ((foundDiff.y1 == 0 && foundDiff.x1 >= 0 && size.width - foundDiff.x2 < obsdiff) || (foundDiff.y2 == 0 && foundDiff.x1 >= 0 && size.width - foundDiff.x2 < obsdiff)) {
            rDiff = 1;
        } else if ((foundDiff.y1 == 0 && foundDiff.x2 >= 0 && size.width - foundDiff.x1 < obsdiff) || (foundDiff.y2 == 0 && foundDiff.x2 >= 0 && size.width - foundDiff.x1 < obsdiff)) {
            rDiff = -1;
        }
    } else {
        diff.first = foundDiff.x1;
        diff.second = foundDiff.x2;

        int32_t obsdiff = size.height / 2;
        if ((foundDiff.x1 == 0 && foundDiff.y1 >= 0 && size.height - foundDiff.y2 < obsdiff) || (foundDiff.x2 == 0 && foundDiff.y1 >= 0 && size.height - foundDiff.y2 < obsdiff)) {
            rDiff = 1;
        } else if ((foundDiff.x1 == 0 && foundDiff.y2 >= 0 && size.height - foundDiff.y1 < obsdiff) || (foundDiff.x2 == 0 && foundDiff.y2 >= 0 && size.height - foundDiff.y1 < obsdiff)) {
            rDiff = -1;
        }
    }

    return found;
}

uint32_t Map::tilesCount(TilesetAbove::Type above)
{
    uint32_t ret = 0;
    for (const auto& layer : mLayers) {
        ret += layer->tilesCount(above);
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
