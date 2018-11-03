#include <fstream>

#include <json/json.h>

#include <engine/logger.h>
#include <engine/assetmanager.h>
#include <engine/image.h>
#include <engine/tileset.h> 

using namespace engine;

TilesetType::TilesetType(Types::Dimension& tileDimension, const std::string& tileType, const std::string& nodeType, int& x, int& y, int& typeHeight, int frames)
	: mValid(false)
	, mTileDimension(tileDimension)
	, mTileType(TileTypeSingle)
	, mNodeType(TileNodeTypeNone)
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
		eCritical() << "Invalid tileset node tile" << tileType;
		return;
	}

	if(nodeType == "wall") {
		mNodeType = TileNodeTypeWall;
	} else if(nodeType == "water") {
		mNodeType = TileNodeTypeWater;
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

TilesetNode* TilesetType::toNode(std::map<int, std::map<int, int> > &tiles, uint32_t x, uint32_t y, uint32_t width, uint32_t height)
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

bool TilesetType::isSolid() const
{
	return mNodeType == TileNodeTypeWall
		|| mNodeType == TileNodeTypeWater;
}

Tileset::Tileset(const std::string& name)
	: mValid(false)
	, mTileDimension({32, 32})
	, mImage(nullptr)
{
	std::shared_ptr<Json::Value> docPtr = AssetManager::data(AssetManager::Tileset, name);
	Json::Value doc = *docPtr;
	if(!doc.isObject()) {
		eCritical() << "Could not open tileset JSON file" << name;
		return;
	}

	if(!doc.isMember("image")|| !doc.isMember("nodes")) {
		eCritical() << "Could not find required tileset JSON data for" << name;
		return;
	}

	mImage = AssetManager::image(AssetManager::Tileset, doc["image"].asString());
	if(!*mImage) {
		eCritical() << "Could not load tileset image for" << name;
		return;
	}
	
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
			eCritical() << "Could not find required tileset node JSON data for" << name;
			return;
		}

		if(y >= mImage->height()) {
			eCritical() << "Too many nodes in tileset data for" << name;
			return;
		}

		int frames = 0;
		if(nodeObj.isMember("animation") && nodeObj["animation"].asBool() && nodeObj.isMember("frames")) {
			frames = nodeObj["frames"].asInt();
		}

		std::string nodeType = "";
		if(nodeObj.isMember("type")) {
			nodeType = nodeObj["type"].asString();
		}

		TilesetType* type = new TilesetType(mTileDimension, nodeObj["tile"].asString(), nodeType, x, y, typeHeight, frames);
		if(!*type) {
			delete type;
			return;
		}

		if(x >= mImage->width()) {
			x = 0;
			y += typeHeight;
			typeHeight = mTileDimension.height;
		}

		mTypes[i] = std::shared_ptr<TilesetType>(type);
	}

	if(y != mImage->height()) {
		eWarning() << "Possible missing nodes in tileset data for" << name;
	}

	mValid = true;
}

Tileset::~Tileset()
{

}

void Tileset::draw(Renderer& renderer, TilesetNode& node, const Types::Point& pos)
{
	int dx = 0, dy = 0;
	for(int i = 0; i < 4; i++) {
		Types::Rect dst((pos.x * mTileDimension.width) + (dx * (mTileDimension.width / 2)), (pos.y * mTileDimension.height) + (dy * (mTileDimension.height / 2)), mTileDimension.width / 2, mTileDimension.height / 2);
		Types::Rect src(node.pos[i].x + (node.anim.x * node.current), node.pos[i].y + (node.anim.y * node.current), mTileDimension.width / 2, mTileDimension.height / 2);
		renderer.drawImage(*mImage, dst, src);

		dx++;
		if(dx > 1) {
			dx = 0;
			dy++;
		}
	}
}

bool Tileset::updateTiles(std::map<int, std::map<int, int> > &table, std::map<int, std::shared_ptr<TilesetNode> >& map, uint32_t width, uint32_t height)
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

bool Tileset::isSolid(int n) const 
{
	return n >= 0 && n < static_cast<int>(mTypes.size()) && mTypes.at(n)->isSolid();
}
