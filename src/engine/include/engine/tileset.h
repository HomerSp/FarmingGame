#pragma once

#include <unordered_map>
#include <memory>

#include <png++/png.hpp>

#include <engine/renderer.h>
#include <engine/collisionmap.h>

namespace engine {
	struct TilesetNode {
		Types::Point pos[4];
		Types::Point anim;
		int frames;
		int current;
	};

	struct TilesetAttribute
	{
	public:
		typedef enum
		{
			RowAbove = 0,
			Water,

			Start = RowAbove,
			End = Water,
		} Type;
	};
	

	class TilesetType {
	public:
		TilesetType(Types::Dimension& tileDimension, const std::string& tileType, const std::unordered_map<TilesetAttribute::Type, bool>& attrs, int& x, int& y, int& typeHeight, int frames);

		TilesetNode* toNode(std::map<int, std::map<int, int> > &tiles, uint32_t x, uint32_t y, uint32_t width, uint32_t height);

		bool hasAttribute(TilesetAttribute::Type type) const {
			return mAttributes.find(type) != mAttributes.end();
		}

		bool operator!() const {
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
		std::unordered_map<TilesetAttribute::Type, bool> mAttributes;
		int mX;
		int mY;
		int mFrames;
	};

	class Tileset {
	public:
		Tileset(const std::string& name);
		~Tileset();

		void draw(Renderer& renderer, TilesetNode& node, const Types::Point& pos);

		Types::Dimension getTileDimension() const {
			return mTileDimension;
		}

		std::shared_ptr<CollisionMap> loadCollisionMap();

		void updateCollisionMap(CollisionMap& tilesetMap, CollisionMap& map, TilesetNode& node, uint32_t x, uint32_t y);
		bool updateTiles(std::map<int, std::map<int, int> >& tiles, std::map<int, std::shared_ptr<TilesetNode> >& map, uint32_t width, uint32_t height);

		bool operator!() const {
			return !mValid;
		}

	private:
		bool mValid;
		Types::Dimension mTileDimension;
		std::map<int, std::shared_ptr<TilesetType> > mTypes;
		std::shared_ptr<engine::Image> mImage;
		std::string mCollisionMap;
	};
}
