#pragma once

#include <memory>
#include <unordered_map>
#include <unordered_set>

#include <engine/types.h>
#include <engine/renderer.h>
#include <engine/tileset.h>

namespace engine {
	class MapLayer {
	public:
		MapLayer(const Types::Map2D& data, std::shared_ptr<Tileset> tileset, uint32_t width, uint32_t height);
		~MapLayer();

		void animate(uint64_t frameDiff);
		void draw(Renderer& renderer, const Types::Rect& rect, bool clip);

		bool updateCollisionMap(CollisionMap& map);

		bool operator!() const {
			return !mValid;
		}

	private:
		bool mValid;
		Types::Map2D mData;
		std::shared_ptr<Tileset> mTileset;
		uint32_t mWidth;
		uint32_t mHeight;
		std::unordered_map<int, std::shared_ptr<TilesetNode> > mNodes;
		uint64_t mCurrentFrames;
	};

	class Map {
	public:
		Map(const std::string& name);
		~Map();

		void animate(uint64_t frameDiff);
		void draw(Renderer& renderer, const Types::Rect& rect, bool clip = true);

		void checkCollision(const Types::PointF& pos, const Types::Dimension& size, Types::PointF& dst, Types::PointF& velocity) const;

		uint32_t width() const {
			return mWidth;
		}

		uint32_t pixelWidth() const {
			return mWidth * getTileDimension().width;
		}

		uint32_t height() const {
			return mHeight;
		}

		uint32_t pixelHeight() const {
			return mHeight * getTileDimension().height;
		}

		Types::Dimension getTileDimension() const;

		bool operator!() const {
			return !mValid;
		}

	protected:
		bool isColliding(const Types::PointF& pos, const Types::Dimension& size, Types::Pair& diff, int8_t& rDiff, bool vertical) const;

	private:
		bool mValid;
		std::unordered_map<std::string, std::shared_ptr<Tileset> > mTilesets;
		std::vector<std::shared_ptr<MapLayer> > mLayers;
		std::shared_ptr<CollisionMap> mCollisionMap;
		uint32_t mWidth;
		uint32_t mHeight;
	};
}
