#pragma once

#include <atomic>
#include <bitset>
#include <cmath>
#include <map>
#include <memory>
#include <unordered_map>
#include <unordered_set>

#include <engine/types.h>

namespace engine {

class CollisionMap;
class Image;
class Renderer;
class TilesetType;
class Tileset;

struct TilesetNode {
    Types::Point<> pos[4];
    Types::Point<> anim;
    uint8_t frames;
    std::atomic<float_t> current;
    uint32_t toggleWidth;
    bool toggled;
    TilesetType* type;
};

struct TilesetAttribute {
    typedef enum {
        Water = 0,
        Toggle,
        LightSource,
        Path,

        Last,
    } Type;
};

struct TilesetAbove {
    typedef enum {
        None = 0,
        Row,
        All,
    } Type;
};

class TilesetType {
public:
    TilesetType(uint32_t index, Types::Dimension<>& tileDimension, const std::string& tileType, uint32_t x, uint32_t y);

    bool checkBase(Types::Map2D& tiles, TilesetAbove::Type above, uint32_t x, uint32_t y);
    std::shared_ptr<TilesetNode> toNode(Types::Map2D& tiles, uint32_t x, uint32_t y, uint32_t width, uint32_t height);

    bool contains(const Types::Point<uint32_t>& other) const;
    bool hasAttribute(TilesetAttribute::Type type) const;

    TilesetAbove::Type above() const;
    uint32_t index() const;

    Types::Point<> lightBase() const;
    int32_t lightRadius() const;
    float_t lightStrength() const;

    bool operator!() const;

protected:
    void setAbove(TilesetAbove::Type above);
    void setAttributes(const std::bitset<TilesetAttribute::Last>& attrs);
    void setBase(uint32_t base);
    void setCount(Types::Cells count);
    void setFrames(int8_t frames);
    void setLightBase(Types::Point<> base);
    void setLightRadius(int32_t radius);
    void setLightStrength(float_t strength);

private:
    friend class Tileset;

    enum TileType {
        TileTypeSingle = 0,
        TileTypeAuto,
        TileTypeAutoHoriz,
    };

    bool mValid;
    uint32_t mIndex;
    Types::Dimension<>& mTileDimension;
    TileType mTileType;
    TilesetAbove::Type mTileAbove;
    std::bitset<TilesetAttribute::Last> mAttributes;
    Types::Rect<uint32_t> mSize;
    int32_t mFrames;
    Types::Cells mCount;
    uint32_t mBase;

    // Light
    Types::Point<> mLightBase;
    int32_t mLightRadius;
    float_t mLightStrength;
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
    bool updateTiles(Types::Map2D& tiles, std::map<int32_t, std::map<int32_t, std::shared_ptr<TilesetNode>>>& map, uint32_t width, uint32_t height, TilesetAbove::Type above);

    bool operator!() const
    {
        return !mValid;
    }

private:
    bool mValid;
    Types::Dimension<> mTileDimension;
    std::unordered_map<int32_t, std::shared_ptr<TilesetType>> mTypes;
    std::shared_ptr<engine::Image> mImage;
    std::string mCollisionMap;
};
}
