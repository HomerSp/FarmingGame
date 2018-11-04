#include <fstream>

#include <json/json.h>

#include <engine/logger.h>
#include <engine/assetmanager.h>
#include <engine/image.h>
#include <engine/tileset.h> 

using namespace engine;

TilesetType::TilesetType(Types::Dimension& tileDimension, const std::string& tileType, const std::bitset<TilesetAttribute::Last>& attrs, int& x, int& y, int& typeHeight, int frames)
	: mValid(false)
	, mTileDimension(tileDimension)
	, mTileType(TileTypeSingle)
	, mAttributes(attrs)
	, mX(x)
	, mY(y)
	, mFrames(frames)
{
	if(tileType == "automatic") {
		mTileType = TileTypeAuto;
	} else if(tileType == "automatic_horizontal") {
		mTileType = TileTypeAutoHoriz;
	} else if(tileType == "single") {
		mTileType = TileTypeSingle;
	} else {
		Logger::critical() << "Invalid tileset node tile" << tileType;
		return;
	}

	switch(mTileType) {
	case TileTypeAuto:
		if(frames > 0) {
			x += (mTileDimension.width * 2) * frames;
		} else {
			x += mTileDimension.width * 2;
		}
		typeHeight = 3 * mTileDimension.height;
		break;
	case TileTypeAutoHoriz:
		x += mTileDimension.width * 2;
		if(frames > 0) {
			typeHeight = frames * mTileDimension.height;
		}
		break;
	default:
		x += mTileDimension.width;
		break;
	}

	mValid = true;
}

TilesetNode* TilesetType::toNode(Types::Map2D& tiles, uint32_t x, uint32_t y, uint32_t width, uint32_t height)
{
	TilesetNode* node = new TilesetNode();
	node->anim = Types::Point((mTileType == TileTypeAuto) ? (mTileDimension.width * 2) : 0, (mTileType == TileTypeAutoHoriz) ? mTileDimension.height : 0);
	node->frames = mFrames;
	node->current = 0;

	switch(mTileType) {
	case TileTypeSingle:
		// Top left
		node->pos[0].x = mX;
		node->pos[0].y = mY;

		// Top right
		node->pos[1].x = mX + (mTileDimension.width / 2);
		node->pos[1].y = mY;

		// Bottom left
		node->pos[2].x = mX;
		node->pos[2].y = mY + (mTileDimension.height / 2);

		// Bottom right
		node->pos[3].x = mX + (mTileDimension.width / 2);
		node->pos[3].y = mY + (mTileDimension.height / 2);
		break;
	case TileTypeAuto:
		// Top left
		if(x > 0 && y > 0) {
			if(tiles[x - 1][y - 1] != tiles[x][y] && tiles[x - 1][y] == tiles[x][y] && tiles[x][y - 1] == tiles[x][y]) {
				node->pos[0].x = mX + mTileDimension.width;
				node->pos[0].y = mY;
			} else if(tiles[x - 1][y] != tiles[x][y] && tiles[x][y - 1] != tiles[x][y]) {
				node->pos[0].x = mX;
				node->pos[0].y = mY + mTileDimension.height;
			} else if(tiles[x - 1][y] != tiles[x][y]) {
				node->pos[0].x = mX;
				node->pos[0].y = mY + (mTileDimension.height + (mTileDimension.height / 2));
			} else if(tiles[x][y - 1] != tiles[x][y]) {
				node->pos[0].x = mX + (mTileDimension.width / 2);
				node->pos[0].y = mY + mTileDimension.height;
			} else {
				node->pos[0].x = mX + (mTileDimension.width / 2);
				node->pos[0].y = mY + (mTileDimension.height + (mTileDimension.height / 2));
			}
		} else if(x > 0) {
			if(tiles[x - 1][y] != tiles[x][y]) {
				node->pos[0].x = mX;
				node->pos[0].y = mY + (mTileDimension.height + (mTileDimension.height / 2));
			} else {
				node->pos[0].x = mX + (mTileDimension.width / 2);
				node->pos[0].y = mY + (mTileDimension.height + (mTileDimension.height / 2));
			}
		} else if(y > 0) {
			if(tiles[x][y - 1] != tiles[x][y]) {
				node->pos[0].x = mX + (mTileDimension.width / 2);
				node->pos[0].y = mY + mTileDimension.height;
			} else {
				node->pos[0].x = mX + (mTileDimension.width / 2);
				node->pos[0].y = mY + (mTileDimension.height + (mTileDimension.height / 2));
			}
		} else {
			node->pos[0].x = mX + (mTileDimension.width / 2);
			node->pos[0].y = mY + (mTileDimension.height + (mTileDimension.height / 2));
		}

		// Top right
		if(x < width - 1 && y > 0) {
			if(tiles[x + 1][y - 1] != tiles[x][y] && tiles[x + 1][y] == tiles[x][y] && tiles[x][y - 1] == tiles[x][y]) {
				node->pos[1].x = mX + (mTileDimension.width + (mTileDimension.width / 2));
				node->pos[1].y = mY;
			} else if(tiles[x + 1][y] != tiles[x][y] && tiles[x][y - 1] != tiles[x][y]) {
				node->pos[1].x = mX + (mTileDimension.width + (mTileDimension.width / 2));
				node->pos[1].y = mY + mTileDimension.height;
			} else if(tiles[x + 1][y] != tiles[x][y]) {
				node->pos[1].x = mX + (mTileDimension.width + (mTileDimension.width / 2));
				node->pos[1].y = mY + (mTileDimension.height + (mTileDimension.height / 2));
			} else if(tiles[x][y - 1] != tiles[x][y]) {
				node->pos[1].x = mX + mTileDimension.width;
				node->pos[1].y = mY + mTileDimension.height;
			} else {
				node->pos[1].x = mX + mTileDimension.width;
				node->pos[1].y = mY + (mTileDimension.height + (mTileDimension.height / 2));
			}
		} else if(x < width - 1) {
			if(tiles[x + 1][y] != tiles[x][y]) {
				node->pos[1].x = mX + (mTileDimension.width + (mTileDimension.width / 2));
				node->pos[1].y = mY + (mTileDimension.height + (mTileDimension.height / 2));
			} else {
				node->pos[1].x = mX + mTileDimension.width;
				node->pos[1].y = mY + (mTileDimension.height + (mTileDimension.height / 2));
			}
		} else if(y > 0) {
			if(tiles[x][y - 1] != tiles[x][y]) {
				node->pos[1].x = mX + mTileDimension.width;
				node->pos[1].y = mY + mTileDimension.height;
			} else {
				node->pos[1].x = mX + mTileDimension.width;
				node->pos[1].y = mY + (mTileDimension.height + (mTileDimension.height / 2));
			}
		} else {
			node->pos[1].x = mX + mTileDimension.width;
			node->pos[1].y = mY + (mTileDimension.height + (mTileDimension.height / 2));
		}

		// Bottom left
		if(x > 0 && y < height - 1) {
			if(tiles[x - 1][y + 1] != tiles[x][y] && tiles[x - 1][y] == tiles[x][y] && tiles[x][y + 1] == tiles[x][y]) {
				node->pos[2].x = mX + mTileDimension.width;
				node->pos[2].y = mY + (mTileDimension.height / 2);
			} else if(tiles[x - 1][y] != tiles[x][y] && tiles[x][y + 1] != tiles[x][y]) {
				node->pos[2].x = mX;
				node->pos[2].y = mY + (mTileDimension.height * 2) + (mTileDimension.height / 2);
			} else if(tiles[x - 1][y] != tiles[x][y]) {
				node->pos[2].x = mX;
				node->pos[2].y = mY + (mTileDimension.height * 2);
			} else if(tiles[x][y + 1] != tiles[x][y]) {
				node->pos[2].x = mX + (mTileDimension.width / 2);
				node->pos[2].y = mY + (mTileDimension.height * 2) + (mTileDimension.height / 2);
			} else {
				node->pos[2].x = mX + (mTileDimension.width / 2);
				node->pos[2].y = mY + (mTileDimension.height + (mTileDimension.height / 2));
			}
		} else if(x > 0) {
			if(tiles[x - 1][y] != tiles[x][y]) {
				node->pos[2].x = mX;
				node->pos[2].y = mY + (mTileDimension.height * 2);
			} else {
				node->pos[2].x = mX + (mTileDimension.width / 2);
				node->pos[2].y = mY + (mTileDimension.height + (mTileDimension.height / 2));
			}
		} else if(y < height - 1) {
			if(tiles[x][y + 1] != tiles[x][y]) {
				node->pos[2].x = mX + (mTileDimension.width / 2);
				node->pos[2].y = mY + (mTileDimension.height * 2) + (mTileDimension.height / 2);
			} else {
				node->pos[2].x = mX + (mTileDimension.width / 2);
				node->pos[2].y = mY + (mTileDimension.height + (mTileDimension.height / 2));
			}
		} else {
			node->pos[2].x = mX + (mTileDimension.width / 2);
			node->pos[2].y = mY + (mTileDimension.height + (mTileDimension.height / 2));
		}

		// Bottom right
		if(x < width - 1 && y < height - 1) {
			if(tiles[x + 1][y + 1] != tiles[x][y] && tiles[x + 1][y] == tiles[x][y] && tiles[x][y + 1] == tiles[x][y]) {
				node->pos[3].x = mX + (mTileDimension.width + (mTileDimension.width / 2));
				node->pos[3].y = mY + (mTileDimension.height / 2);
			} else if(tiles[x + 1][y] != tiles[x][y] && tiles[x][y + 1] != tiles[x][y]) {
				node->pos[3].x = mX + (mTileDimension.width + (mTileDimension.width / 2));
				node->pos[3].y = mY + (mTileDimension.height * 2) + (mTileDimension.height / 2);
			} else if(tiles[x + 1][y] != tiles[x][y]) {
				node->pos[3].x = mX + (mTileDimension.width + (mTileDimension.width / 2));
				node->pos[3].y = mY + (mTileDimension.height * 2);
			} else if(tiles[x][y + 1] != tiles[x][y]) {
				node->pos[3].x = mX + mTileDimension.width;
				node->pos[3].y = mY + (mTileDimension.height * 2) + (mTileDimension.height / 2);
			} else {
				node->pos[3].x = mX + mTileDimension.width;
				node->pos[3].y = mY + (mTileDimension.height * 2);
			}
		} else if(x < width - 1) {
			if(tiles[x + 1][y] != tiles[x][y]) {
				node->pos[3].x = mX + (mTileDimension.width + (mTileDimension.width / 2));
				node->pos[3].y = mY + (mTileDimension.height * 2);
			} else {
				node->pos[3].x = mX + mTileDimension.width;
				node->pos[3].y = mY + (mTileDimension.height * 2);
			}
		} else if(y < height - 1) {
			if(tiles[x][y + 1] != tiles[x][y]) {
				node->pos[3].x = mX + mTileDimension.width;
				node->pos[3].y = mY + (mTileDimension.height * 2) + (mTileDimension.height / 2);
			} else {
				node->pos[3].x = mX + mTileDimension.width;
				node->pos[3].y = mY + (mTileDimension.height * 2);
			}
		} else {
			node->pos[3].x = mX + mTileDimension.width;
			node->pos[3].y = mY + (mTileDimension.height * 2);
		}

		break;
	case TileTypeAutoHoriz:
		// Top left
		if(x > 0 && tiles[x - 1][y] != tiles[x][y]) {
			node->pos[0].x = mX;
			node->pos[0].y = mY;
		} else {
			node->pos[0].x = mX + (mTileDimension.width / 2);
			node->pos[0].y = mY;
		}

		// Top right
		if(x < width - 1 && tiles[x + 1][y] != tiles[x][y]) {
			node->pos[1].x = mX + (mTileDimension.width + (mTileDimension.width / 2));
			node->pos[1].y = mY;
		} else {
			node->pos[1].x = mX + mTileDimension.width;
			node->pos[1].y = mY;
		}

		// Bottom left
		if(x > 0 && tiles[x - 1][y] != tiles[x][y]) {
			node->pos[2].x = mX;
			node->pos[2].y = mY + (mTileDimension.height / 2);
		} else {
			node->pos[2].x = mX + (mTileDimension.width / 2);
			node->pos[2].y = mY + (mTileDimension.height / 2);
		}

		// Bottom right
		if(x < width - 1 && tiles[x + 1][y] != tiles[x][y]) {
			node->pos[3].x = mX + (mTileDimension.width + (mTileDimension.width / 2));
			node->pos[3].y = mY + (mTileDimension.height / 2);
		} else {
			node->pos[3].x = mX + mTileDimension.width;
			node->pos[3].y = mY + (mTileDimension.height / 2);
		}

		break;
	}

	return node;
}

Tileset::Tileset(const std::string& name)
	: mValid(false)
	, mTileDimension({32, 32})
	, mImage(nullptr)
{
	std::shared_ptr<Json::Value> docPtr = AssetManager::data(AssetManager::Tileset, name);
	Json::Value doc = *docPtr;
	if(!doc.isObject()) {
		Logger::critical() << "Could not open tileset JSON file" << name;
		return;
	}

	if(!doc.isMember("image")|| !doc.isMember("nodes") || !doc.isMember("collision")) {
		Logger::critical() << "Could not find required tileset JSON data for" << name;
		return;
	}

	mImage = AssetManager::image(AssetManager::Tileset, doc["image"].asString());
	if(!*mImage) {
		Logger::critical() << "Could not load tileset image for" << name;
		return;
	}

	mCollisionMap = doc["collision"].asString();

	if(doc.isMember("dimen")) {
		Json::Value dimenArr = doc["dimen"];
		if(dimenArr.size() == 2) {
			mTileDimension.width = dimenArr[0].asInt();
			mTileDimension.height = dimenArr[1].asInt();
		}
	}

	Json::Value nodes = doc["nodes"];

	int x = 0, y = 0, typeHeight = mTileDimension.height;
	for(uint32_t i = 0; i < nodes.size(); i++) {
		Json::Value nodeObj = nodes[i];
		if(!nodeObj.isMember("tile")) {
			Logger::critical() << "Could not find required tileset node JSON data for" << name;
			return;
		}

		if(y >= mImage->height()) {
			Logger::critical() << "Too many nodes in tileset data for" << name;
			return;
		}

		int frames = 0;
		if(nodeObj.isMember("animation")) {
			Json::Value animObj = nodeObj["animation"];
			if(animObj.isMember("frames")) {
				frames = animObj["frames"].asInt();
			}
		}

		std::bitset<TilesetAttribute::Last> attrs;
		if(nodeObj.isMember("attributes")) {
			Json::Value attrsObj = nodeObj["attributes"];
			for(uint32_t i = 0; i < attrsObj.size(); i++)
			{
				std::string key = attrsObj[i].asString();
				if(key == "water")
				{
					attrs[TilesetAttribute::Water] = true;
				} else if(key == "above_row")
				{
					attrs[TilesetAttribute::AboveRow] = true;
				} else if(key == "above_all")
				{
					attrs[TilesetAttribute::AboveAll] = true;
				} else {
					Logger::warning() << "Unknown attribute" << key << "for tileset" << name;
				}
			}
		}

		std::shared_ptr<TilesetType> type = std::shared_ptr<TilesetType>(new TilesetType(mTileDimension, nodeObj["tile"].asString(), attrs, x, y, typeHeight, frames));
		if(!*type) {
			return;
		}

		if(x >= mImage->width()) {
			x = 0;
			y += typeHeight;
			typeHeight = mTileDimension.height;
		}

		mTypes[i] = type;
	}

	if(y != mImage->height()) {
		Logger::warning() << "Possible missing nodes in tileset data for" << name;
	}

	mValid = true;
}

Tileset::~Tileset()
{

}

void Tileset::draw(Renderer& renderer, TilesetNode& node, const Types::Point& pos)
{
	Types::Rect dst(0, 0, mTileDimension.width / 2, mTileDimension.height / 2);
	int dx = 0, dy = 0;
	for(int i = 0; i < 4; i++) {
		dst.x = (pos.x * mTileDimension.width) + (dx * (mTileDimension.width / 2));
		dst.y = (pos.y * mTileDimension.height) + (dy * (mTileDimension.height / 2));

		Types::Rect src(node.pos[i].x + (node.anim.x * node.current), node.pos[i].y + (node.anim.y * node.current), dst.width, dst.height);
		renderer.drawImage(*mImage, dst, src);

		dx++;
		if(dx > 1) {
			dx = 0;
			dy++;
		}
	}
}

std::shared_ptr<CollisionMap> Tileset::loadCollisionMap()
{
	std::shared_ptr<CollisionMap> collisionMap = AssetManager::collision(AssetManager::Tileset, mCollisionMap);
	if(!*collisionMap) {
		Logger::critical() << "Could not load tileset collision map" << mCollisionMap;
	}

	return collisionMap;
}

void Tileset::updateCollisionMap(CollisionMap& tilesetMap, CollisionMap& map, TilesetNode& node, uint32_t x, uint32_t y)
{
	Types::Dimension dimen(mTileDimension.width / 2, mTileDimension.height / 2);

	uint32_t dx = 0, dy = 0;
	for(int i = 0; i < 4; i++)
	{
		for(int cy = 0; cy < dimen.height; cy++)
		{
			for(int cx = 0; cx < dimen.width; cx++)
			{
				if(tilesetMap.get(node.pos[i].x + cx, node.pos[i].y + cy)) {
					uint32_t dstx = (x * mTileDimension.width) + (dx * (mTileDimension.width / 2)) + cx;
					uint32_t dsty = (y * mTileDimension.height) + (dy * (mTileDimension.height / 2)) + cy;
					map.set(dstx, dsty, true);
				}
			}
		}

		dx++;
		if(dx > 1) {
			dx = 0;
			dy++;
		}
	}
}

bool Tileset::updateTiles(Types::Map2D& table, std::unordered_map<int, std::shared_ptr<TilesetNode> >& map, uint32_t width, uint32_t height)
{
	for(uint32_t x = 0; x < width; x++) {
		for(uint32_t y = 0; y < height; y++) {
			int n = table[x][y];
			if(n == 0) {
				continue;
			}

			TilesetNode* node = mTypes[n - 1]->toNode(table, x, y, width, height);
			map[(y * width) + x] = std::shared_ptr<TilesetNode>(node);
		}
	}

	return true;
}
