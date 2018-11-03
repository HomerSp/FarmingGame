#pragma once

#include <memory>

#include <png++/png.hpp>

#include <engine/renderer.h>

namespace engine {
	struct TilesetNode {
		Types::Point pos[4];
		Types::Point anim;
		int frames;
		int current;
	};

	class TilesetType {
	public:
		TilesetType(Types::Dimension& tileDimension, const std::string& tileType, const std::string& nodeType, int& x, int& y, int& typeHeight, int frames);

		TilesetNode* toNode(std::map<int, std::map<int, int> > &tiles, uint32_t x, uint32_t y, uint32_t width, uint32_t height);

		bool isSolid() const;

		bool operator!() const {
			return !mValid;
		}

	private:
		enum TileType {
			TileTypeSingle = 0,
			TileTypeAuto,
			TileTypeAutoHoriz,
		};

		enum TileNodeType {
			TileNodeTypeNone = 0,
			TileNodeTypeWall,
			TileNodeTypeWater,
		};

		bool mValid;
		Types::Dimension& mTileDimension;
		TileType mTileType;
		TileNodeType mNodeType;
		int mX;
		int mY;
		int mFrames;
	};

	class Tileset {
	public:
		Tileset(const std::string& name);
		~Tileset();

		void draw(Renderer& renderer, TilesetNode& node, const Types::Point& pos);

		bool updateTiles(std::map<int, std::map<int, int> >& tiles, std::map<int, std::shared_ptr<TilesetNode> >& map, uint32_t width, uint32_t height);

		Types::Dimension getTileDimension() const {
			return mTileDimension;
		}

		bool isSolid(int n) const ;

		bool operator!() const {
			return !mValid;
		}

	private:
		bool mValid;
		Types::Dimension mTileDimension;
		std::map<int, std::shared_ptr<TilesetType> > mTypes;
		std::shared_ptr<engine::Image> mImage;
	};
}
