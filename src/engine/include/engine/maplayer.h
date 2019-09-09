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
class BufferWriter;
class Renderer;
}

class MapLayer {
public:
    MapLayer(graphics::Renderer& renderer, Types::Map2D data, std::shared_ptr<Tileset> tileset, uint32_t width, uint32_t height, uint32_t tilesetIndex);

    bool animate(uint64_t frameDiff);

    void draw(graphics::Renderer& renderer, const Types::Rect<>& dst, TilesetAbove::Type above, bool clip);
    void drawRow(graphics::Renderer& renderer, const Types::Rect<>& dst, int32_t row, TilesetAbove::Type above, bool clip);
    uint32_t drawBuffer(graphics::Renderer& renderer, const Types::Rect<>& dst, TilesetAbove::Type above, graphics::BufferWriter& writer, bool clip);
    uint32_t drawRowBuffer(graphics::Renderer& renderer, const Types::Rect<>& dst, int32_t row, TilesetAbove::Type above, graphics::BufferWriter& writer, bool clip);

    void toggleLights(bool on);

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
