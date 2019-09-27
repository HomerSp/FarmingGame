#pragma once

#include <cmath>
#include <map>
#include <memory>
#include <unordered_map>
#include <unordered_set>

#include <engine/collisionmap.h>
#include <engine/contextobject.h>
#include <engine/graphics/colorgradient.h>
#include <engine/overlay.h>
#include <engine/tileset.h>
#include <engine/types.h>

namespace engine {

class Context;
class MapLightSource;
class Tileset;

namespace graphics {
class Buffer;
class Renderer;
}

class MapLayer {
public:
    MapLayer(graphics::Renderer& renderer, Types::Map2D data, std::shared_ptr<Tileset> tileset, uint32_t width, uint32_t height, uint32_t tilesetIndex);

    void updateBuffer(graphics::Renderer& renderer, TilesetAbove::Type above, graphics::Buffer::Writer& writer);
    void updateRowBuffer(graphics::Renderer& renderer, int32_t row, TilesetAbove::Type above, graphics::Buffer::Writer& writer);

    uint32_t tilesCount(TilesetAbove::Type above);

    bool toggleLights(bool on);

    bool updateCollisionMap(CollisionMap& outMap);
    bool updateLightSources(std::vector<std::shared_ptr<MapLightSource>>& sources);
    bool updatePaths(std::vector<Types::Point<int32_t> > &paths);

    bool operator!() const
    {
        return !mValid;
    }

private:
    bool mValid;
    Types::Map2D mData;
    std::shared_ptr<Tileset> mTileset;
    uint32_t mTilesetIndex;
    Types::Dimension<uint32_t> mDimensions;

    std::map<TilesetAbove::Type, std::map<int32_t, std::map<int32_t, std::shared_ptr<TilesetNode>>>> mNodes;
    std::vector<TilesetNode*> mLightNodes;
};
}
