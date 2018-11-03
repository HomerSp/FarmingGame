#pragma once

#include <memory>
#include <list>

#include <engine/renderer.h>
#include <engine/tileset.h>

namespace engine {
	class MapLayer {
	public:
		MapLayer(const std::map<int, std::map<int, int> >& data, std::shared_ptr<Tileset> tileset, uint32_t width, uint32_t height);
		~MapLayer();

		void animate(uint64_t frameDiff);
		void draw(Renderer& renderer, const Types::Rect& rect, bool clip);

		void updateCollisionList(std::map<uint32_t, bool> &list);

		bool operator!() const {
			return !mValid;
		}

	private:
		bool mValid;
		std::map<int, std::map<int, int> > mData;
		std::shared_ptr<Tileset> mTileset;
		uint32_t mWidth;
		uint32_t mHeight;
		std::map<int, std::shared_ptr<TilesetNode> > mNodes;
		uint64_t mCurrentFrames;
	};

	class Map {
	public:
		Map(const std::string& name);
		~Map();

		void animate(uint64_t frameDiff);
		void draw(Renderer& renderer, const Types::Rect& rect, bool clip = true);

		void checkCollision(const Types::PointF& pos, const Types::Point& size, Types::PointF& dst, Types::PointF& velocity) const;

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
		bool isColliding(const Types::PointF& pos, const Types::Point& size, int8_t& diff, bool vertical) const;

	private:
		bool mValid;
		std::map<std::string, std::shared_ptr<Tileset> > mTilesets;
		std::list<std::shared_ptr<MapLayer> > mLayers;
		std::map<uint32_t, bool> mCollisionList;
		uint32_t mWidth;
		uint32_t mHeight;
	};
};
