#include <algorithm>
#include <cmath>

#include <json/value.h>

#include <engine/collisionmap.h>
#include <engine/graphics/renderer.h>
#include <engine/logger.h>
#include <engine/map.h>
#include <engine/maplayer.h>

using namespace engine;

MapLayer::MapLayer(graphics::Renderer& renderer, Types::Map2D data, std::shared_ptr<Tileset> tileset, uint32_t width, uint32_t height, uint32_t tilesetIndex)
    : mValid(false)
    , mData(std::move(data))
    , mTileset(std::move(tileset))
    , mTilesetIndex(tilesetIndex)
    , mDimensions(width, height)
{
    // Add water nodes
    if (!mTileset->updateTiles(mData, mNodes[TilesetAbove::Water], mDimensions.width, mDimensions.height, TilesetAbove::Water)) {
        return;
    }

    // Add ground nodes
    if (!mTileset->updateTiles(mData, mNodes[TilesetAbove::None], mDimensions.width, mDimensions.height, TilesetAbove::None)) {
        return;
    }

    // Add tree nodes and such
    if (!mTileset->updateTiles(mData, mNodes[TilesetAbove::Row], mDimensions.width, mDimensions.height, TilesetAbove::Row)) {
        return;
    }

    // Add nodes that are always above characters
    if (!mTileset->updateTiles(mData, mNodes[TilesetAbove::All], mDimensions.width, mDimensions.height, TilesetAbove::All)) {
        return;
    }

    // Add light nodes
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

void MapLayer::updateBuffer(graphics::Renderer& renderer, TilesetAbove::Type above, graphics::Buffer::Writer& writer)
{
    for (auto &nodeY: mNodes[above]) {
        updateRowBuffer(renderer, nodeY.first, above, writer);
    }
}

void MapLayer::updateRowBuffer(graphics::Renderer& renderer, int32_t row, TilesetAbove::Type above, graphics::Buffer::Writer& writer)
{
    auto* nodes = &mNodes[above];

    // No nodes at this row, return.
    if (nodes->find(row) == nodes->end()) {
        return;
    }

    float_t zOrder = 1.0f;
    if (above == TilesetAbove::All) {
        zOrder = -1.0f;
    }

    auto nodeRow = nodes->at(row);
    for (auto &node : nodeRow) {
        if (above == TilesetAbove::Row) {
            zOrder = 1.0f - ((node.second->baseY) / static_cast<float_t>(mDimensions.height));
        }

        mTileset->updateBuffer(renderer, *node.second, { node.first, row }, writer, mTilesetIndex, zOrder);
    }
}

uint32_t MapLayer::tilesCount(TilesetAbove::Type above)
{
    uint32_t ret = 0;
    for (auto &nodeY: mNodes[above]) {
        ret += nodeY.second.size() * 4;
    }

    return ret;
}

bool MapLayer::toggleLights(bool on)
{
    bool changed = false;
    for (auto node: mLightNodes) {
        if (node->toggled == on) {
            continue;
        }

        node->toggled = on;
        changed = true;
    }

    return changed;
}

bool MapLayer::updateCollisionMap(CollisionMap& outMap)
{
    std::unique_ptr<CollisionMap> tilesetCollisionMap = mTileset->loadCollisionMap();
    if (!tilesetCollisionMap) {
        Logger::error("MapLayer") << "Could not load tileset collision map";
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
                if (node->type->hasAttribute(TilesetAttribute::LightSource) && std::find(added.begin(), added.end(), node->id) == added.end()) {
                    Types::Point<> base = node->type->lightBase();
                    auto dst = Types::Point<int32_t>(base.x + nodeX.first * d.width, base.y + nodeY.first * d.height);
                    std::shared_ptr<MapLightSource> s = std::make_shared<MapLightSource>(dst, node->type->lightRadius(), node->type->lightColor());
                    sources.push_back(s);
                    
                    added.push_back(node->id);
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
