#pragma once

#include <cmath>
#include <map>
#include <memory>
#include <unordered_map>
#include <unordered_set>

#include <engine/collisionmap.h>
#include <engine/contextobject.h>
#include <engine/graphics/buffer.h>
#include <engine/graphics/colorgradient.h>
#include <engine/graphics/texture.h>
#include <engine/overlay.h>
#include <engine/tileset.h>
#include <engine/types.h>

namespace engine {

class Context;
class MapLayer;
class Tileset;

namespace graphics {
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

    void drawBuffer(graphics::Renderer& renderer, const Types::Point<>& dst, TilesetAbove::Type above);
    void updateBuffers(graphics::Renderer& renderer);

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
    std::pair<bool, bool> isColliding(const Types::Point<uint32_t>& pos, const Types::Point<>& dst, const Types::Dimension<>& size, Types::Quad<>& diff, Types::Pair& diffPos) const;

    uint32_t tilesCount(TilesetAbove::Type above);

private:
    bool mValid;
    std::string mID;
    Map::Type::Val mType;
    std::unordered_map<std::string, std::shared_ptr<Tileset>> mTilesets;
    std::unordered_map<std::string, uint32_t> mTilesetIndexes;
    std::vector<std::shared_ptr<MapLayer>> mLayers;
    std::unique_ptr<CollisionMap> mCollisionMap;
    Types::Dimension<uint32_t> mDimensions;
    std::vector<std::shared_ptr<MapLightSource>> mLights;
    std::vector<Types::Point<int32_t >> mPaths;

    std::atomic<bool> mNeedUpdate;
    std::unordered_map<TilesetAbove::Type, std::unique_ptr<graphics::Buffer>> mBuffer;
    std::unordered_map<TilesetAbove::Type, uint32_t> mBufferCount;
    std::unique_ptr<graphics::Texture> mTexture;

    std::atomic<float_t> mAnimFrame;
};
}
