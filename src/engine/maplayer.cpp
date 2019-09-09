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

void MapLayer::draw(graphics::Renderer& renderer, const Types::Rect<>& dst, TilesetAbove::Type above, bool clip)
{
    for (auto &nodeY: mNodes[above]) {
        if (nodeY.first >= dst.y - 1 && nodeY.first <= dst.y + dst.height + 1) {
            drawRow(renderer, dst, nodeY.first, above, clip);
        }
    }
}

void MapLayer::drawRow(graphics::Renderer& renderer, const Types::Rect<>& dst, int32_t row, TilesetAbove::Type above, bool clip)
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

uint32_t MapLayer::drawBuffer(graphics::Renderer& renderer, const Types::Rect<>& dst, TilesetAbove::Type above, graphics::BufferWriter& writer, bool clip)
{
    uint32_t ret = 0;
    for (auto &nodeY: mNodes[above]) {
        if (nodeY.first >= dst.y - 1 && nodeY.first <= dst.y + dst.height + 1) {
            ret += drawRowBuffer(renderer, dst, nodeY.first, above, writer, clip);
        }
    }

    return ret;
}

uint32_t MapLayer::drawRowBuffer(graphics::Renderer& renderer, const Types::Rect<>& dst, int32_t row, TilesetAbove::Type above, graphics::BufferWriter& writer, bool clip)
{
    auto* nodes = &mNodes[above];

    // No nodes at this row, return.
    if (nodes->find(row) == nodes->end()) {
        return 0;
    }

    uint32_t ret = 0;

    auto nodeRow = nodes->at(row);
    for (auto &node : nodeRow) {
        if (!clip || (node.first >= dst.x - 1 && node.first <= dst.x + dst.width + 1)) {
            ret += mTileset->drawBuffer(renderer, *node.second, { node.first - dst.x, row - dst.y }, writer, mTilesetIndex);
        }
    }

    return ret;
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
