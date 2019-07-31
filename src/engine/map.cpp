#include <algorithm>
#include <cmath>
#include <fstream>

#include <json/json.h>

#include <engine/assetmanager.h>
#include <engine/collisionmap.h>
#include <engine/logger.h>
#include <engine/map.h>
#include <engine/renderer.h>

using namespace engine;

MapLayer::MapLayer(Types::Map2D data, std::shared_ptr<Tileset> tileset, uint32_t width, uint32_t height)
    : mValid(false)
    , mData(std::move(data))
    , mTileset(std::move(tileset))
    , mDimensions(width, height)
{
    if (!mTileset->updateTiles(mData, mNodes[TilesetAbove::None], mDimensions.width, mDimensions.height, TilesetAbove::None)) {
        return;
    }

    if (!mTileset->updateTiles(mData, mNodes[TilesetAbove::Row], mDimensions.width, mDimensions.height, TilesetAbove::Row)) {
        return;
    }

    if (!mTileset->updateTiles(mData, mNodes[TilesetAbove::All], mDimensions.width, mDimensions.height, TilesetAbove::All)) {
        return;
    }

    for (auto& above: mNodes) {
        for(auto &nodeY: above.second) {
            for (auto &nodeX: nodeY.second) {
                if(nodeX.second->toggleWidth > 0) {
                    mLightNodes.push_back(nodeX.second.get());
                }
            }
        }
    }
    
    mValid = true;
}

bool MapLayer::animate(uint64_t frameDiff)
{
    bool changed = false;
    for (auto& above: mNodes) {
        for (auto &nodeY: above.second) {
            for (auto &nodeX: nodeY.second) {
                if (nodeX.second->frames > 0) {
                    float_t c = nodeX.second->current;
                    c += frameDiff / 200.0f;
                    if (c >= nodeX.second->frames) {
                        c = 0;
                    }

                    changed = std::floor(c) != std::floor(nodeX.second->current);
                    nodeX.second->current = c;
                }
            }
        }
    }

    return changed;
}

void MapLayer::draw(Renderer& renderer, const Types::Rect<>& dst, bool clip)
{
    for (auto &nodeY: mNodes[TilesetAbove::None]) {
        if (nodeY.first >= dst.y - 1 && nodeY.first <= dst.y + dst.height + 1) {
            drawRow(renderer, dst, nodeY.first, TilesetAbove::None, clip);
        }
    }
}

void MapLayer::drawRow(Renderer& renderer, const Types::Rect<>& dst, int32_t row, TilesetAbove::Type above, bool clip)
{
    auto* nodes = &mNodes[above];

    // No nodes at this row, return.
    if (nodes->find(row) == nodes->end()) {
        return;
    }

    auto nodeRow = nodes->at(row);
    for (auto &node : nodeRow) {
        if (!clip || (node.first >= dst.x - 1 && node.first <= dst.x + dst.width + 1)) {
            mTileset->draw(renderer, *node.second, { node.first - dst.x, row - dst.y });
        }
    }
}

void MapLayer::toggleLights(bool on)
{
    for (auto node: mLightNodes) {
        node->toggled = on;
    }
}

bool MapLayer::updateCollisionMap(CollisionMap& outMap)
{
    std::unique_ptr<CollisionMap> tilesetCollisionMap = mTileset->loadCollisionMap();
    if (!tilesetCollisionMap) {
        Logger::error() << "Could not load tileset collision map";
        return false;
    }

    for(auto &nodeY: mNodes[TilesetAbove::None]) {
        for (auto &nodeX: nodeY.second) {
            mTileset->updateCollisionMap(*tilesetCollisionMap, outMap, *nodeX.second, nodeX.first, nodeY.first);
        }
    }

    for(auto &nodeY: mNodes[TilesetAbove::Row]) {
        for (auto &nodeX: nodeY.second) {
            mTileset->updateCollisionMap(*tilesetCollisionMap, outMap, *nodeX.second, nodeX.first, nodeY.first);
        }
    }

    return true;
}

bool MapLayer::updateLightSources(std::vector<std::shared_ptr<MapLightSource>>& sources)
{
    Types::Dimension<> d = mTileset->getTileDimension();

    std::vector<uint32_t> added;
    for (auto& above: mNodes) {
        for(auto &nodeY: above.second) {
            for (auto &nodeX: nodeY.second) {
                TilesetNode* node = nodeX.second.get();
                if (node->type->hasAttribute(TilesetAttribute::LightSource) && std::find(added.begin(), added.end(), node->type->index()) == added.end()) {
                    Types::Point<> base = node->type->lightBase();
                    auto dst = Types::Point<int32_t>(base.x + nodeX.first * d.width, base.y + nodeY.first * d.height);
                    std::shared_ptr<MapLightSource> s = std::make_shared<MapLightSource>(dst, node->type->lightRadius(), node->type->lightStrength());
                    sources.push_back(s);
                    
                    added.push_back(node->type->index());
                }
            }
        }
    }

    return true;
}

bool MapLayer::updatePaths(std::vector<Types::Point<int32_t> > &paths)
{
    for (auto& above: mNodes) {
        for(auto &nodeY: above.second) {
            for (auto &nodeX: nodeY.second) {
                TilesetNode* node = nodeX.second.get();
                if (node->type->hasAttribute(TilesetAttribute::Path)) {
                    auto dst = Types::Point<int32_t>(nodeX.first, nodeY.first);
                    paths.push_back(dst);
                }
            }
        }
    }

    return true;
}

Map::Map(std::shared_ptr<Context>& ctx, const std::string& id)
    : ContextObject(ctx)
    , mValid(false)
    , mID(id)
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

        std::shared_ptr<MapLayer> layer = std::make_shared<MapLayer>(data, mTilesets.find(name)->second, mDimensions.width, mDimensions.height);
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

void Map::drawRow(Renderer& renderer, const Types::Rect<>& dst, int32_t row, TilesetAbove::Type above, bool clip)
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

void Map::addLightSources(std::vector<std::shared_ptr<ScreenEffects::LightSource>>& sources)
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

MapLightSource::MapLightSource(Types::Point<int32_t> pos, int32_t radius, float_t strength)
    : mPosition(pos)
    , mRadius(radius)
    , mStrength(strength)
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

float_t MapLightSource::lightStrength()
{
    return mStrength;
}
