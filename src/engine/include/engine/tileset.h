#pragma once

#include <atomic>
#include <bitset>
#include <memory>
#include <unordered_map>
#include <unordered_set>

#include <engine/types.h>

namespace engine {

class CollisionMap;
class Image;
class Renderer;

struct TilesetNode {
public:
    Types::Point<> pos[4];
    Types::Point<> anim;
    int frames;
    std::atomic<float> current;
    uint32_t toggleWidth;
    bool toggled;
};

struct TilesetAttribute {
public:
    typedef enum {
        AboveNone = 0,
        AboveRow,
        AboveAll,
        Water,
        Toggle,
        LightSource,

        Last,
    } Type;
};

class TilesetType {
public:
    TilesetType(Types::Dimension<>& tileDimension, const std::string& tileType, const std::bitset<TilesetAttribute::Last>& attrs, uint32_t x, uint32_t y, int frames, Types::Cells count, uint32_t base);

    bool checkBase(Types::Map2D& tiles, TilesetAttribute::Type type, uint32_t x, uint32_t y);
    std::shared_ptr<TilesetNode> toNode(Types::Map2D& tiles, uint32_t x, uint32_t y, uint32_t width, uint32_t height);

    bool contains(const Types::Point<uint32_t>& other) const;
    bool hasAttribute(TilesetAttribute::Type type) const;

    bool operator!() const;

private:
    enum TileType {
        TileTypeSingle = 0,
        TileTypeAuto,
        TileTypeAutoHoriz,
    };

    bool mValid;
    Types::Dimension<>& mTileDimension;
    TileType mTileType;
    std::bitset<TilesetAttribute::Last> mAttributes;
    Types::Rect<uint32_t> mSize;
    int mFrames;
    Types::Cells mCount;
    uint32_t mBase;
};

class Tileset {
public:
    Tileset(const std::string& name);

    void draw(Renderer& renderer, TilesetNode& node, const Types::Point<>& pos);

    Types::Dimension<> getTileDimension() const
    {
        return mTileDimension;
    }

    std::shared_ptr<CollisionMap> loadCollisionMap();

    void updateCollisionMap(CollisionMap& tilesetMap, CollisionMap& map, TilesetNode& node, uint32_t x, uint32_t y);
    bool updateTiles(Types::Map2D& tiles, std::unordered_map<int, std::unordered_map<int, std::shared_ptr<TilesetNode>>>& map, uint32_t width, uint32_t height, TilesetAttribute::Type type);

    bool operator!() const
    {
        return !mValid;
    }

private:
    bool mValid;
    Types::Dimension<> mTileDimension;
    std::unordered_map<int, std::shared_ptr<TilesetType>> mTypes;
    std::shared_ptr<engine::Image> mImage;
    std::string mCollisionMap;
};
}
