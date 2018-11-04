#pragma once

#include <bitset>
#include <memory>
#include <unordered_map>
#include <unordered_set>

#include <png++/png.hpp>

#include <engine/collisionmap.h>
#include <engine/renderer.h>
#include <engine/types.h>

namespace engine {
struct TilesetNode {
public:
    Types::Point pos[4];
    Types::Point anim;
    int frames;
    int current;
};

struct TilesetAttribute {
public:
    typedef enum {
        AboveRow = 0,
        AboveAll,
        Water,
        Last,
    } Type;
};

class TilesetType {
public:
    TilesetType(Types::Dimension& tileDimension, const std::string& tileType, const std::bitset<TilesetAttribute::Last>& attrs, uint32_t& x, uint32_t& y, uint32_t& typeHeight, int frames);

    std::shared_ptr<TilesetNode> toNode(Types::Map2D& tiles, uint32_t x, uint32_t y, uint32_t width, uint32_t height);

    bool hasAttribute(TilesetAttribute::Type type) const
    {
        return mAttributes[type];
    }

    bool operator!() const
    {
        return !mValid;
    }

private:
    enum TileType {
        TileTypeSingle = 0,
        TileTypeAuto,
        TileTypeAutoHoriz,
    };

    bool mValid;
    Types::Dimension& mTileDimension;
    TileType mTileType;
    std::bitset<TilesetAttribute::Last> mAttributes;
    uint32_t mX;
    uint32_t mY;
    int mFrames;
};

class Tileset {
public:
    Tileset(const std::string& name);

    void draw(Renderer& renderer, TilesetNode& node, const Types::Point& pos);

    Types::Dimension getTileDimension() const
    {
        return mTileDimension;
    }

    std::shared_ptr<CollisionMap> loadCollisionMap();

    void updateCollisionMap(CollisionMap& tilesetMap, CollisionMap& map, TilesetNode& node, uint32_t x, uint32_t y);
    bool updateTiles(Types::Map2D& tiles, std::unordered_map<int, std::shared_ptr<TilesetNode>>& map, uint32_t width, uint32_t height);

    bool operator!() const
    {
        return !mValid;
    }

private:
    bool mValid;
    Types::Dimension mTileDimension;
    std::unordered_map<int, std::shared_ptr<TilesetType>> mTypes;
    std::shared_ptr<engine::Image> mImage;
    std::string mCollisionMap;
};
}
