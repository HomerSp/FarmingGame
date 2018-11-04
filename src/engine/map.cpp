#include <cmath>
#include <fstream>

#include <json/json.h>

#include <engine/assetmanager.h>
#include <engine/logger.h>
#include <engine/map.h>

using namespace engine;

MapLayer::MapLayer(Types::Map2D data, std::shared_ptr<Tileset> tileset, uint32_t width, uint32_t height)
    : mValid(false)
    , mData(std::move(data))
    , mTileset(std::move(tileset))
    , mWidth(width)
    , mHeight(height)
    , mCurrentFrames(0)
{
    mValid = mTileset->updateTiles(mData, mNodes, mWidth, mHeight);
}

void MapLayer::animate(uint64_t currentFrame)
{
    for (auto node : mNodes) {
        if (node.second->frames > 0) {
            node.second->current = std::floor(currentFrame % (200 * node.second->frames) / 200);
        }
    }
}

void MapLayer::draw(Renderer& renderer, const Types::Rect& dst, bool clip)
{
    for (auto node : mNodes) {
        int nx = node.first % mWidth;
        int ny = std::floor(node.first / mWidth);
        if (!clip || (clip && nx >= dst.x - 1 && ny >= dst.y - 1 && nx <= dst.x + dst.width + 1 && ny <= dst.y + dst.height + 1)) {
            mTileset->draw(renderer, *node.second, { nx - dst.x, ny - dst.y });
        }
    }
}

bool MapLayer::updateCollisionMap(CollisionMap& map)
{
    std::shared_ptr<CollisionMap> tilesetCollisionMap = mTileset->loadCollisionMap();
    if (!*tilesetCollisionMap) {
        return false;
    }

    for (auto node : mNodes) {
        uint32_t nx = node.first % mWidth;
        uint32_t ny = std::floor(node.first / mWidth);
        mTileset->updateCollisionMap(*tilesetCollisionMap, map, *(node.second), nx, ny);
    }

    return true;
}

Map::Map(const std::string& name)
    : mValid(false)
    , mWidth(0)
    , mHeight(0)
{
    Logger::debug() << "Loading Map" << name;

    std::shared_ptr<Json::Value> docPtr = AssetManager::get()->data(AssetManager::get()->Map, name);
    Json::Value doc = *docPtr;
    if (!doc.isObject()) {
        Logger::critical() << "Invalid JSON data for" << name;
        return;
    }

    if (!doc.isMember("width") || !doc.isMember("height") || !doc.isMember("layers")) {
        Logger::critical() << "Could not find required map JSON attributes for" << name;
        return;
    }

    mWidth = doc["width"].asInt();
    mHeight = doc["height"].asInt();

    Json::Value layers = doc["layers"];
    for (auto layerObj : layers) {
        if (!layerObj.isMember("tileset")) {
            Logger::critical() << "Could not find required map layer JSON attributes for" << name;
            return;
        }

        std::string name = layerObj["tileset"].asString();
        if (mTilesets.find(name) != mTilesets.end()) {
            continue;
        }

        mTilesets[name] = std::make_shared<Tileset>(name);
        if (!*mTilesets[name]) {
            Logger::critical() << "Could not load tileset for" << name;
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
            Logger::critical() << "Could not find required layer JSON attributes for" << name;
            return;
        }

        std::unordered_map<int, std::unordered_map<int, int>> data;
        Json::Value dataObj = layerObj["data"];
        for (uint32_t x = 0; x < mWidth; x++) {
            for (uint32_t y = 0; y < mHeight; y++) {
                data[x][y] = dataObj[x + (y * mWidth)].asInt();
            }
        }

        std::shared_ptr<MapLayer> layer = std::make_shared<MapLayer>(data, mTilesets.find(name)->second, mWidth, mHeight);
        if (!*layer) {
            Logger::critical() << "Could not load layer for" << name;
            return;
        }

        mLayers.push_back(layer);
    }

    mCollisionMap = std::make_shared<CollisionMap>(pixelWidth(), pixelHeight());
    mCollisionMap->set(144, 0, true);
    for (const auto& layer : mLayers) {
        if (!layer->updateCollisionMap(*mCollisionMap.get())) {
            Logger::critical() << "Could not load collision map for" << name;
            return;
        }
    }

    mValid = true;
}

void Map::animate(uint64_t currentFrame)
{
    for (const auto& layer : mLayers) {
        layer->animate(currentFrame);
    }
}

void Map::draw(Renderer& renderer, const Types::Rect& dst, bool clip)
{
    Types::Dimension tileDimens = getTileDimension();
    Types::Rect target;
    target.x = std::ceil(dst.x / tileDimens.width);
    target.y = std::ceil(dst.y / tileDimens.height);
    target.width = std::ceil(dst.width / tileDimens.width);
    target.height = std::ceil(dst.height / tileDimens.height);

    renderer.translate(-(dst.x % tileDimens.width), -(dst.y % tileDimens.height));
    for (const auto& layer : mLayers) {
        layer->draw(renderer, target, clip);
    }
    renderer.translate((dst.x % tileDimens.width), (dst.y % tileDimens.height));
}

void Map::checkCollision(const Types::PointF& pos, const Types::Dimension& size, Types::PointF& dst, Types::PointF& velocity) const
{
    if (pos.x + dst.x < 0.0f) {
        dst.x = 0.0f;
        velocity.x = 0.0f;
    }

    if (pos.y + dst.y < -std::floor(size.height / 2)) {
        dst.y = 0.0f;
        velocity.y = 0.0f;
    }

    // No need to continue checking.
    if (dst.x == 0.0f && dst.y == 0.0f) {
        return;
    }

    Types::Pair diff;
    int8_t rDiff = 0;
    if (dst.x != 0.0f && isColliding({ pos.x + dst.x, pos.y }, size, diff, rDiff, false)) {
        if (rDiff == 0) {
            velocity.x = 0.0f;
        }

        if (dst.y == 0.0f) {
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
        if (rDiff == 0) {
            velocity.y = 0.0f;
        }

        if (dst.x == 0.0f) {
            if (rDiff < 0) {
                dst.x = (dst.y < 0.0f) ? dst.y : -dst.y;
            } else if (rDiff > 0) {
                dst.x = (dst.y < 0.0f) ? -dst.y : dst.y;
            }
        }

        if (dst.y < 0.0f) {
            dst.y = diff.first;
        } else {
            dst.y = diff.second;
        }
    }
}

bool Map::isColliding(const Types::PointF& pos, const Types::Dimension& size, Types::Pair& diff, int8_t& rDiff, bool vertical) const
{
    rDiff = 0;

    int startY = (size.height / 2);
    Types::Quad foundDiff;
    bool found = mCollisionMap->get(pos.x, pos.y + startY, size.width, startY, &foundDiff);

    // Check if we can move around the obstacle.
    if (vertical) {
        diff.first = foundDiff.y1;
        diff.second = foundDiff.y2;

        int obsdiff = size.width / 2;
        if ((foundDiff.y1 == 0 && foundDiff.x1 >= 0 && size.width - foundDiff.x2 < obsdiff) || (foundDiff.y2 == 0 && foundDiff.x1 >= 0 && size.width - foundDiff.x2 < obsdiff)) {
            rDiff = 1;
        } else if ((foundDiff.y1 == 0 && foundDiff.x2 >= 0 && size.width - foundDiff.x1 < obsdiff) || (foundDiff.y2 == 0 && foundDiff.x2 >= 0 && size.width - foundDiff.x1 < obsdiff)) {
            rDiff = -1;
        }
    } else {
        diff.first = foundDiff.x1;
        diff.second = foundDiff.x2;

        int obsdiff = size.height / 4;
        if ((foundDiff.x1 == 0 && foundDiff.y1 >= 0 && startY - foundDiff.y2 < obsdiff) || (foundDiff.x2 == 0 && foundDiff.y1 >= 0 && startY - foundDiff.y2 < obsdiff)) {
            rDiff = 1;
        } else if ((foundDiff.x1 == 0 && foundDiff.y2 >= 0 && startY - foundDiff.y1 < obsdiff) || (foundDiff.x2 == 0 && foundDiff.y2 >= 0 && startY - foundDiff.y1 < obsdiff)) {
            rDiff = -1;
        }
    }

    return found;
}

Types::Dimension Map::getTileDimension() const
{
    for (auto tileset : mTilesets) {
        return tileset.second->getTileDimension();
    }

    return { 32, 32 };
}