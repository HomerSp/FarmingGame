#include <cmath>
#include <fstream>

#include <json/json.h>

#include <engine/logger.h>
#include <engine/assetmanager.h>
#include <engine/map.h>

using namespace engine;

MapLayer::MapLayer(const std::map<int, std::map<int, int> >& data, std::shared_ptr<Tileset> tileset, uint32_t width, uint32_t height)
	: mValid(false)
	, mData(data)
	, mTileset(tileset)
	, mWidth(width)
	, mHeight(height)
	, mCurrentFrames(0)
{
	mValid = mTileset->updateTiles(mData, mNodes, mWidth, mHeight);
}

MapLayer::~MapLayer()
{

}

void MapLayer::animate(uint64_t currentFrame) {
	for(auto it = mNodes.begin(); it != mNodes.end(); it++) {
		if(it->second->frames > 0) {
			it->second->current = std::floor(currentFrame % (200 * it->second->frames) / 200);
		}
	}
}

void MapLayer::draw(Renderer& renderer, const Types::Rect& dst, bool clip)
{
	for(auto it = mNodes.begin(); it != mNodes.end(); it++) {
		int nx = it->first % mWidth;
		int ny = std::floor(it->first / mWidth);
		if(!clip || (clip && nx >= dst.x - 1 && ny >= dst.y - 1 && nx <= dst.x + dst.width + 1 && ny <= dst.y + dst.height + 1)) {
			mTileset->draw(renderer, *it->second, {nx - dst.x, ny - dst.y});
		}
	}
}

void MapLayer::updateCollisionList(std::map<uint32_t, bool> &list)
{
	for(uint32_t x = 0; x < mData.size(); x++) {
		for(uint32_t y = 0; y < mData.at(x).size(); y++) {
			if(mTileset->isSolid(mData.at(x).at(y) - 1)) {
				list[x + (y * mData.size())] = true;
			}
		}
	}
}

Map::Map(const std::string& name)
	: mValid(false)
	, mWidth(0)
	, mHeight(0)
{
	std::shared_ptr<Json::Value> docPtr = AssetManager::data(AssetManager::Map, name);
	Json::Value doc = *docPtr;
	if(!doc.isObject()) {
		eCritical() << "Invalid JSON data for" << name;
		return;
	}

	if(!doc.isMember("width") || !doc.isMember("height") || !doc.isMember("layers")) {
		eCritical() << "Could not find required map JSON attributes for" << name;
		return;
	}

	mWidth = doc["width"].asInt();
	mHeight = doc["height"].asInt();

	Json::Value layers = doc["layers"];
	for(uint32_t i = 0; i < layers.size(); i++) {
		Json::Value layerObj = layers[i];
		if(!layerObj.isMember("tileset")) {
			eCritical() << "Could not find required map layer JSON attributes for" << name;
			return;
		}

		std::string name = layerObj["tileset"].asString();
		if(mTilesets.find(name) != mTilesets.end()) {
			continue;
		}

		std::shared_ptr<Tileset> tileset = std::shared_ptr<Tileset>(new Tileset(name));
		if(!*tileset) {
			return;
		}

		mTilesets[name] = tileset;
	}

	for(uint32_t i = 0; i < layers.size(); i++) {
		Json::Value layerObj = layers[i];
		std::string name = layerObj["tileset"].asString();
		if(mTilesets.find(name) == mTilesets.end()) {
			eCritical() << "Could not find tileset for" << name;
			return;
		}

		if(!layerObj.isMember("data")) {
			eCritical() << "Could not find required layer JSON attributes for" << name;
			return;
		}

		std::map<int, std::map<int, int> > data;
		Json::Value dataObj = layerObj["data"];
		for(uint32_t x = 0; x < mWidth; x++) {
			for(uint32_t y = 0; y < mHeight; y++) {
				data[x][y] = dataObj[x + (y * mWidth)].asInt();
			}
		}

		std::shared_ptr<MapLayer> layer = std::shared_ptr<MapLayer>(new MapLayer(data, mTilesets.find(name)->second, mWidth, mHeight));
		if(!*layer) {
			return;
		}

		layer->updateCollisionList(mCollisionList);

		mLayers.push_back(std::shared_ptr<MapLayer>(layer));
	}

	mValid = true;
}

Map::~Map()
{

}

void Map::animate(uint64_t frameDiff) {
	for(auto it = mLayers.begin(); it != mLayers.end(); it++)
	{
		(*it)->animate(frameDiff);
	}
}

void Map::draw(Renderer& renderer, const Types::Rect& dst, bool clip) {
	Types::Dimension tileDimens = getTileDimension();
	Types::Rect target;
	target.x = std::ceil(dst.x / tileDimens.width);
	target.y = std::ceil(dst.y / tileDimens.height);
	target.width = std::ceil(dst.width / tileDimens.width);
	target.height = std::ceil(dst.height / tileDimens.height);

	renderer.translate(-(dst.x % tileDimens.width), -(dst.y % tileDimens.height));
	for(auto it = mLayers.begin(); it != mLayers.end(); it++)
	{
		(*it)->draw(renderer, target, clip);
	}
	renderer.translate((dst.x % tileDimens.width), (dst.y % tileDimens.height));
}

void Map::checkCollision(const Types::PointF& pos, const Types::Point& size, Types::PointF& dst, Types::PointF& velocity) const
{
	if(pos.x + dst.x < 0.0f)
	{
		dst.x = 0.0f;
		velocity.x = 0.0f;
	}

	if(pos.y + dst.y < -(size.y / 2))
	{
		dst.y = 0.0f;
		velocity.y = 0.0f;
	}

	// No need to continue checking.
	if(dst.x == 0.0f && dst.y == 0.0f)
	{
		return;
	}

	int8_t diff = 0;
	if(dst.x != 0.0f && isColliding({pos.x + dst.x, pos.y}, size, diff, false)) {
		if(diff == 0) {
			velocity.x = 0.0f;
		}

		if(dst.y == 0.0f) {
			if(diff < 0) {
				dst.y = (dst.x < 0.0f) ? dst.x : -dst.x;
			} else if(diff > 0) {
				dst.y = (dst.x < 0.0f) ? -dst.x : dst.x;
			}
		}

		Types::Dimension d = getTileDimension();
		if(dst.x < 0.0f) {
			dst.x = (std::floor(pos.x / d.width) * d.width) - pos.x + 0.01f;
		} else {
			dst.x = (std::floor(pos.x / d.width) * d.width) + d.width - pos.x - 0.01f;
		}
	}
	if(dst.y != 0.0f && isColliding({pos.x, pos.y + dst.y}, size, diff, true)) {
		if(diff == 0) {
			velocity.y = 0.0f;
		}

		if(dst.x == 0.0f) {
			if(diff < 0) {
				dst.x = (dst.y < 0.0f) ? dst.y : -dst.y;
			} else if(diff > 0) {
				dst.x = (dst.y < 0.0f) ? -dst.y : dst.y;
			}
		}

		Types::Dimension d = getTileDimension();
		if(dst.y < 0.0f) {
			dst.y = (std::floor(pos.y / d.height) * d.height) + (size.y / 2) - pos.y + 0.01f;
		} else {
			dst.y = (std::floor(pos.y / d.height) * d.height) + d.height - pos.y - 0.01f;
		}
	}
}

bool Map::isColliding(const Types::PointF& pos, const Types::Point& size, int8_t& diff, bool vertical) const
{
	diff = 0;

	Types::Dimension d = getTileDimension();

	uint32_t xLeft = (pos.x) / d.width;
	uint32_t xRight = (pos.x + size.x) / d.width;	
	uint32_t yTop = (pos.y + (size.y / 2)) / d.height;
	uint32_t yBottom = (pos.y + size.y) / d.height;
	if(xRight >= mWidth || yBottom >= mHeight) {
		return true;
	}

	bool topLeft = mCollisionList.find(xLeft + (yTop * mWidth)) != mCollisionList.end();
	bool topRight = mCollisionList.find(xRight + (yTop * mWidth)) != mCollisionList.end();
	bool bottomLeft = mCollisionList.find(xLeft + (yBottom * mWidth)) != mCollisionList.end();
	bool bottomRight = mCollisionList.find(xRight + (yBottom * mWidth)) != mCollisionList.end();

	// Check if we can move around the obstacle.
	if(vertical) {
		if((topLeft && !topRight) || (bottomLeft && !bottomRight)) {
			if(pos.x >= (xRight * d.width) - (size.x / 2)) {
				diff = 1;
			}
		} else if((!topLeft && topRight) || (!bottomLeft && bottomRight)) {
			if(pos.x <= (xLeft * d.width) + (size.x / 2)) {
				diff = -1;
			}
		}
	} else {
		if((topRight && !bottomRight) || (topLeft && !bottomLeft)) {
			if(pos.y >= (yBottom * d.height) - (size.y) + (size.y / 4)) {
				diff = 1;
			}
		} else if((bottomRight && !topRight) || (bottomLeft && !topLeft)) {
			if(pos.y <= (yTop * d.height) + (size.y / 4)) {
				diff = -1;
			}
		}
	}
	
	return topLeft
		|| topRight
		|| bottomLeft
		|| bottomRight;
}

Types::Dimension Map::getTileDimension() const {
	for(auto it = mTilesets.begin(); it != mTilesets.end(); it++) {
		return it->second->getTileDimension();
	}

	return Types::Dimension(32, 32);
}