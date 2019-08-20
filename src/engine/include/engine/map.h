#pragma once

#include <cmath>
#include <map>
#include <memory>
#include <unordered_map>
#include <unordered_set>

#include <engine/collisionmap.h>
#include <engine/context.h>
#include <engine/screeneffects.h>
#include <engine/tileset.h>
#include <engine/types.h>

namespace engine {

class Tileset;

namespace graphics {
class Renderer;
}

class MapLightSource : public ScreenEffects::LightSource {
public:
    MapLightSource(Types::Point<int32_t> pos, int32_t radius, float_t strength, const graphics::ColorGradient& color);
    virtual ~MapLightSource() = default;

    virtual Types::Point<int32_t> lightPosition();
    virtual int32_t lightRadius();
    virtual float_t lightStrength();
    virtual graphics::ColorGradient lightColor() override;

private:
    Types::Point<int32_t> mPosition;
    int32_t mRadius;
    float_t mStrength;
    graphics::ColorGradient mColor;
};

class MapLayer {
public:
    MapLayer(Types::Map2D data, std::shared_ptr<Tileset> tileset, uint32_t width, uint32_t height);

    bool animate(uint64_t frameDiff);

    void draw(graphics::Renderer& renderer, const Types::Rect<>& dst, TilesetAbove::Type above, bool clip);
    void drawRow(graphics::Renderer& renderer, const Types::Rect<>& dst, int32_t row, TilesetAbove::Type above, bool clip);

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
    Types::Dimension<uint32_t> mDimensions;

    std::map<TilesetAbove::Type, std::map<int32_t, std::map<int32_t, std::shared_ptr<TilesetNode>>>> mNodes;
    std::vector<TilesetNode*> mLightNodes;
};

struct MapType {
    typedef enum {
        Outside = 0,
        Inside,
    } Type;
};

class Map : public ContextObject {
public:
    Map(std::shared_ptr<Context>& ctx, const std::string& id);

    bool animate(uint64_t frameDiff);

    void draw(graphics::Renderer& renderer, const Types::Rect<>& dst, TilesetAbove::Type above, bool clip = true);
    void drawRow(graphics::Renderer& renderer, const Types::Rect<>& dst, int32_t row, TilesetAbove::Type above, bool clip = true);

    void checkCollision(const Types::Point<float_t>& pos, const Types::Dimension<>& size, Types::Point<float_t>& dst, float_t& velocityX, float_t& velocityY) const;
    bool isNodeSolid(int32_t x, int32_t y, const Types::Dimension<>& size) const;
    bool isNodePath(int32_t x, int32_t y) const;

    void addLightSources(std::vector<std::shared_ptr<ScreenEffects::LightSource>>& sources);

    void toggleLights(bool on);

    const std::string& id() const;
    MapType::Type type() const;

    uint32_t width() const;
    uint32_t pixelWidth() const;

    uint32_t height() const;
    uint32_t pixelHeight() const;

    Types::Dimension<> getTileDimension() const;

    bool operator!() const;

protected:
    bool isColliding(const Types::Point<float_t>& pos, const Types::Dimension<>& size, Types::Pair& diff, int8_t& rDiff, bool vertical) const;

private:
    bool mValid;
    std::string mID;
    MapType::Type mType;
    std::unordered_map<std::string, std::shared_ptr<Tileset>> mTilesets;
    std::vector<std::shared_ptr<MapLayer>> mLayers;
    std::unique_ptr<CollisionMap> mCollisionMap;
    Types::Dimension<uint32_t> mDimensions;
    std::vector<std::shared_ptr<MapLightSource>> mLights;
    std::vector<Types::Point<int32_t >> mPaths;
};
}
