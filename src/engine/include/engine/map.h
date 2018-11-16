#pragma once

#include <map>
#include <memory>
#include <unordered_map>
#include <unordered_set>

#include <engine/screeneffects.h>
#include <engine/tileset.h>
#include <engine/types.h>

namespace engine {

class Renderer;
class Tileset;

class MapLightSource : public ScreenEffects::LightSource {
public:
    MapLightSource(Types::Point<int32_t> pos, int32_t radius, float strength);

    virtual Types::Point<int32_t> position();
    virtual int32_t radius();
    virtual float strength();

private:
    Types::Point<int32_t> mPosition;
    int32_t mRadius;
    float mStrength;
};

class MapLayer {
public:
    MapLayer(Types::Map2D data, std::shared_ptr<Tileset> tileset, uint32_t width, uint32_t height);

    bool animate(float frameDiff);

    void draw(Renderer& renderer, const Types::Rect<>& dst, bool clip);
    void drawRow(Renderer& renderer, const Types::Rect<>& dst, int row, TilesetAttribute::Type type, bool clip);

    void toggleLights(bool on);

    bool updateCollisionMap(CollisionMap& map);
    bool updateLightSources(std::vector<std::shared_ptr<MapLightSource>>& sources);

    bool operator!() const
    {
        return !mValid;
    }

protected:
	bool updateLightSources(std::vector<std::shared_ptr<MapLightSource>>& sources, std::map<int, std::map<int, std::shared_ptr<TilesetNode>>>& nodes, std::vector<uint32_t> &added);

private:
    bool mValid;
    Types::Map2D mData;
    std::shared_ptr<Tileset> mTileset;
    Types::Dimension<uint32_t> mDimensions;

    std::map<int, std::map<int, std::shared_ptr<TilesetNode>>> mNodes;
    std::map<int, std::map<int, std::shared_ptr<TilesetNode>>> mAboveRowNodes;
    std::map<int, std::map<int, std::shared_ptr<TilesetNode>>> mAboveAllNodes;
    std::vector<TilesetNode*> mLightNodes;
};

class Map {
public:
    Map(const std::string& name);

    bool animate(float frameDiff);

    void draw(Renderer& renderer, const Types::Rect<>& dst, bool clip = true);
    void drawRow(Renderer& renderer, const Types::Rect<>& dst, int row, TilesetAttribute::Type type, bool clip = true);

    void checkCollision(const Types::Point<float>& pos, const Types::Dimension<>& size, Types::Point<float>& dst, float& velocityX, float& velocityY) const;

    void addLightSources(std::vector<std::shared_ptr<ScreenEffects::LightSource>>& sources);

    void toggleLights(bool on);

    uint32_t width() const;
    uint32_t pixelWidth() const;

    uint32_t height() const;
    uint32_t pixelHeight() const;

    Types::Dimension<> getTileDimension() const;

    bool operator!() const;

protected:
    bool isColliding(const Types::Point<float>& pos, const Types::Dimension<>& size, Types::Pair& diff, int8_t& rDiff, bool vertical) const;

private:
    bool mValid;
    std::unordered_map<std::string, std::shared_ptr<Tileset>> mTilesets;
    std::vector<std::shared_ptr<MapLayer>> mLayers;
    std::shared_ptr<CollisionMap> mCollisionMap;
    Types::Dimension<uint32_t> mDimensions;
    std::vector<std::shared_ptr<MapLightSource>> mLights;
};
}
