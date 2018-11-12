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
    , mDimensions(width, height)
{
    if (!mTileset->updateTiles(mData, mNodes, mDimensions.width, mDimensions.height, TilesetAttribute::AboveNone)) {
        return;
    }

    if (!mTileset->updateTiles(mData, mAboveRowNodes, mDimensions.width, mDimensions.height, TilesetAttribute::AboveRow)) {
        return;
    }

    if (!mTileset->updateTiles(mData, mAboveAllNodes, mDimensions.width, mDimensions.height, TilesetAttribute::AboveAll)) {
        return;
    }

    mValid = true;
}

bool MapLayer::animate(float frameDiff)
{
    bool changed = false;
    for(auto nodeY: mNodes) {
        for (auto nodeX: nodeY.second) {
            if (nodeX.second->frames > 0) {
                float c = nodeX.second->current;
                c += 5.0f * frameDiff;
                if (c >= nodeX.second->frames) {
                    c = 0;
                }

                changed = std::floor(c) != std::floor(nodeX.second->current);
                nodeX.second->current = c;
            }
        }
    }
    return changed;
}

void MapLayer::draw(Renderer& renderer, const Types::Rect<>& dst, bool clip)
{
    for (auto nodeY: mNodes) {
        if (nodeY.first >= dst.y - 1 && nodeY.first <= dst.y + dst.height + 1) {
            drawRow(renderer, dst, nodeY.first, TilesetAttribute::AboveNone, clip);
        }
    }
}

void MapLayer::drawRow(Renderer& renderer, const Types::Rect<>& dst, int row, TilesetAttribute::Type type, bool clip)
{
    auto* nodes = &mNodes;
    switch(type) {
    case TilesetAttribute::AboveNone:
        break;
    case TilesetAttribute::AboveRow:
        nodes = &mAboveRowNodes;
        break;
    case TilesetAttribute::AboveAll:
        nodes = &mAboveAllNodes;
        break;
    default:
        return;
    }

    // No nodes at this row, return.
    if (nodes->find(row) == nodes->end()) {
        return;
    }

    auto nodeRow = nodes->at(row);
    for (auto node : nodeRow) {
        if (!clip || (node.first >= dst.x - 1 && node.first <= dst.x + dst.width + 1)) {
            mTileset->draw(renderer, *node.second, { node.first - dst.x, row - dst.y });
        }
    }
}

bool MapLayer::updateCollisionMap(CollisionMap& map)
{
    std::shared_ptr<CollisionMap> tilesetCollisionMap = mTileset->loadCollisionMap();
    if (!*tilesetCollisionMap) {
        return false;
    }

    for(auto nodeY: mNodes) {
        for (auto nodeX: nodeY.second) {
            mTileset->updateCollisionMap(*tilesetCollisionMap, map, *nodeX.second, nodeX.first, nodeY.first);
        }
    }

    for(auto nodeY: mAboveRowNodes) {
        for (auto nodeX: nodeY.second) {
            mTileset->updateCollisionMap(*tilesetCollisionMap, map, *nodeX.second, nodeX.first, nodeY.first);
        }
    }

    return true;
}

Map::Map(const std::string& name)
    : mValid(false)
    , mDimensions(0, 0)
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

    mDimensions.width = doc["width"].asInt();
    mDimensions.height = doc["height"].asInt();

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
        for (uint32_t x = 0; x < mDimensions.width; x++) {
            for (uint32_t y = 0; y < mDimensions.height; y++) {
                data[x][y] = dataObj[x + (y * mDimensions.width)].asInt();
            }
        }

        std::shared_ptr<MapLayer> layer = std::make_shared<MapLayer>(data, mTilesets.find(name)->second, mDimensions.width, mDimensions.height);
        if (!*layer) {
            Logger::critical() << "Could not load layer for" << name;
            return;
        }

        mLayers.push_back(layer);
    }

    mCollisionMap = std::make_shared<CollisionMap>(pixelWidth(), pixelHeight());
    for (const auto& layer : mLayers) {
        if (!layer->updateCollisionMap(*mCollisionMap.get())) {
            Logger::critical() << "Could not load collision map for" << name;
            return;
        }
    }

    mValid = true;
}

bool Map::animate(float frameDiff)
{
    bool changed = false;
    for (const auto& layer : mLayers) {
        if (layer->animate(frameDiff)) {
            changed = true;
        }
    }
    return changed;
}

void Map::draw(Renderer& renderer, const Types::Rect<>& dst, bool clip)
{
    Types::Dimension<> tileDimens = getTileDimension();
    Types::Rect<> target;
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

void Map::drawRow(Renderer& renderer, const Types::Rect<>& dst, int row, TilesetAttribute::Type type, bool clip)
{
    Types::Dimension<> tileDimens = getTileDimension();
    Types::Rect<> target;
    target.x = std::ceil(dst.x / tileDimens.width);
    target.y = std::ceil(dst.y / tileDimens.height);
    target.width = std::ceil(dst.width / tileDimens.width);
    target.height = std::ceil(dst.height / tileDimens.height);

    renderer.translate(-(dst.x % tileDimens.width), -(dst.y % tileDimens.height));
    for (const auto& layer : mLayers) {
        layer->drawRow(renderer, target, row, type, clip);
    }
    renderer.translate((dst.x % tileDimens.width), (dst.y % tileDimens.height));
}

void Map::checkCollision(const Types::Point<float>& pos, const Types::Dimension<>& size, Types::Point<float>& dst, float& velocityX, float& velocityY) const
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

bool Map::isColliding(const Types::Point<float>& pos, const Types::Dimension<>& size, Types::Pair& diff, int8_t& rDiff, bool vertical) const
{
    rDiff = 0;

    Types::Quad<> foundDiff;
    bool found = mCollisionMap->get(pos.x, pos.y, size.width, size.height, &foundDiff);

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

        int obsdiff = size.height / 2;
        if ((foundDiff.x1 == 0 && foundDiff.y1 >= 0 && size.height - foundDiff.y2 < obsdiff) || (foundDiff.x2 == 0 && foundDiff.y1 >= 0 && size.height - foundDiff.y2 < obsdiff)) {
            rDiff = 1;
        } else if ((foundDiff.x1 == 0 && foundDiff.y2 >= 0 && size.height - foundDiff.y1 < obsdiff) || (foundDiff.x2 == 0 && foundDiff.y2 >= 0 && size.height - foundDiff.y1 < obsdiff)) {
            rDiff = -1;
        }
    }

    return found;
}

Types::Dimension<> Map::getTileDimension() const
{
    for (auto tileset : mTilesets) {
        return tileset.second->getTileDimension();
    }

    return { 32, 32 };
}