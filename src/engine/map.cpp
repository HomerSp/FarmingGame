#include <algorithm>
#include <cmath>

#include <json/value.h>

#include <engine/assetmanager.h>
#include <engine/collisionmap.h>
#include <engine/context.h>
#include <engine/graphics/renderer.h>
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
{
    Logger::debug() << "Loading Map" << id;

    auto doc = context().assetManager().data(AssetManager::Map, id);
    if (!doc || !doc->isObject()) {
        Logger::critical() << "Invalid JSON data for" << id;
        return;
    }

    if (!doc->isMember("width") || !doc->isMember("height") || !doc->isMember("layers")) {
        Logger::critical() << "Could not find required map JSON attributes for" << id;
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

    Json::Value layers = (*doc)["layers"];
    for (auto layerObj : layers) {
        if (!layerObj.isMember("tileset")) {
            Logger::critical() << "Could not find required map layer JSON attributes for" << id;
            return;
        }

        std::string name = layerObj["tileset"].asString();
        if (mTilesets.find(name) != mTilesets.end()) {
            continue;
        }

        mTilesets[name] = std::make_shared<Tileset>(contextPtr(), name);
        if (!*mTilesets[name]) {
            Logger::critical() << "Could not load tileset for" << id;
            return;
        }
    }

    for (auto layerObj : layers) {
        std::string name = layerObj["tileset"].asString();
        if (mTilesets.find(name) == mTilesets.end()) {
            Logger::critical() << "Could not find tileset for" << name;
            return;
        }

        if (!layerObj.isMember("data")) {
            Logger::critical() << "Could not find required layer JSON attributes for" << id;
            return;
        }

        std::unordered_map<int32_t, std::unordered_map<int32_t, int32_t>> data;
        Json::Value dataObj = layerObj["data"];
        for (uint32_t x = 0; x < mDimensions.width; x++) {
            for (uint32_t y = 0; y < mDimensions.height; y++) {
                data[x][y] = dataObj[x + (y * mDimensions.width)].asInt();
            }
        }

        std::shared_ptr<MapLayer> layer = std::make_shared<MapLayer>(renderer, data, mTilesets.find(name)->second, mDimensions.width, mDimensions.height);
        if (!*layer) {
            Logger::critical() << "Could not load layer for" << id;
            return;
        }

        mLayers.push_back(layer);
    }

    mCollisionMap = std::make_unique<CollisionMap>(pixelWidth(), pixelHeight());
    for (const auto& layer : mLayers) {
        if (!layer->updateCollisionMap(*mCollisionMap)) {
            Logger::warning() << "Could not load collision map for" << id;
        }

        if (!layer->updateLightSources(mLights)) {
            Logger::warning() << "Could not load light sources for" << id;
        }

        if (!layer->updatePaths(mPaths)) {
            Logger::warning() << "Could not load paths for" << id;
        }
    }

    mValid = true;
}

bool Map::animate(uint64_t frameDiff)
{
    bool changed = false;
    for (const auto& layer : mLayers) {
        if (layer->animate(frameDiff)) {
            changed = true;
        }
    }
    return changed;
}

void Map::draw(graphics::Renderer& renderer, const Types::Rect<>& dst, TilesetAbove::Type above, bool clip)
{
    Types::Dimension<> tileDimens = getTileDimension();
    Types::Rect<> target;
    target.x = std::ceil(dst.x / tileDimens.width);
    target.y = std::ceil(dst.y / tileDimens.height);
    target.width = std::ceil(dst.width / tileDimens.width);
    target.height = std::ceil(dst.height / tileDimens.height);

    renderer.translate(-(dst.x % tileDimens.width), -(dst.y % tileDimens.height));
    for (const auto& layer : mLayers) {
        layer->draw(renderer, target, above, clip);
    }
    renderer.translate((dst.x % tileDimens.width), (dst.y % tileDimens.height));
}

void Map::drawRow(graphics::Renderer& renderer, const Types::Rect<>& dst, int32_t row, TilesetAbove::Type above, bool clip)
{
    Types::Dimension<> tileDimens = getTileDimension();
    Types::Rect<> target;
    target.x = std::ceil(dst.x / tileDimens.width);
    target.y = std::ceil(dst.y / tileDimens.height);
    target.width = std::ceil(dst.width / tileDimens.width);
    target.height = std::ceil(dst.height / tileDimens.height);

    renderer.translate(-(dst.x % tileDimens.width), -(dst.y % tileDimens.height));
    for (const auto& layer : mLayers) {
        layer->drawRow(renderer, target, row, above, clip);
    }
    renderer.translate((dst.x % tileDimens.width), (dst.y % tileDimens.height));
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
        layer->toggleLights(on);
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
