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
class MapLayer;
class Tileset;

namespace graphics {
class Buffer;
class BufferWriter;
class Renderer;
}

class MapLightSource : public Overlay::LightSource {
public:
    MapLightSource(Types::Point<int32_t> pos, int32_t radius, const graphics::ColorGradient& color);
    virtual ~MapLightSource() = default;

    virtual Types::Point<int32_t> lightPosition() override;
    virtual int32_t lightRadius() override;
    virtual graphics::ColorGradient lightColor() override;

private:
    Types::Point<int32_t> mPosition;
    int32_t mRadius;
    graphics::ColorGradient mColor;
};

class Map : public ContextObject {
public:
    struct Type {
        typedef enum {
            Outside = 0,
            Inside,
        } Val;
    };

public:
    Map(std::shared_ptr<Context>& ctx, graphics::Renderer& renderer, const std::string& id);

    bool animate(uint64_t frameDiff);

    void draw(graphics::Renderer& renderer, const Types::Rect<>& dst, TilesetAbove::Type above, bool clip = true);
    void drawRow(graphics::Renderer& renderer, const Types::Rect<>& dst, int32_t row, TilesetAbove::Type above, bool clip = true);

    void checkCollision(const Types::Point<float_t>& pos, const Types::Dimension<>& size, Types::Point<float_t>& dst, float_t& velocityX, float_t& velocityY) const;
    bool isNodeSolid(int32_t x, int32_t y, const Types::Dimension<>& size) const;
    bool isNodePath(int32_t x, int32_t y) const;

    void addLightSources(std::vector<std::shared_ptr<Overlay::LightSource>>& sources);

    void toggleLights(bool on);

    const std::string& id() const;
    Map::Type::Val type() const;

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
    Map::Type::Val mType;
    std::unordered_map<std::string, std::shared_ptr<Tileset>> mTilesets;
    std::vector<std::shared_ptr<MapLayer>> mLayers;
    std::unique_ptr<CollisionMap> mCollisionMap;
    Types::Dimension<uint32_t> mDimensions;
    std::vector<std::shared_ptr<MapLightSource>> mLights;
    std::vector<Types::Point<int32_t >> mPaths;
};
}
