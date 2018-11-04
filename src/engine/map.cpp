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

bool MapLayer::updateCollisionMap(CollisionMap& map)
{
	std::shared_ptr<CollisionMap> tilesetCollisionMap = mTileset->loadCollisionMap();
	if(!*tilesetCollisionMap) {
		return false;
	}

	for(auto it = mNodes.begin(); it != mNodes.end(); it++) {
		uint32_t nx = it->first % mWidth;
		uint32_t ny = std::floor(it->first / mWidth);
		mTileset->updateCollisionMap(*tilesetCollisionMap, map, *(it->second), nx, ny);
	}

	return true;
}

Map::Map(const std::string& name)
	: mValid(false)
	, mWidth(0)
	, mHeight(0)
{
	std::shared_ptr<Json::Value> docPtr = AssetManager::data(AssetManager::Map, name);
	Json::Value doc = *docPtr;
	if(!doc.isObject()) {
		Logger::critical() << "Invalid JSON data for" << name;
		return;
	}

	if(!doc.isMember("width") || !doc.isMember("height") || !doc.isMember("layers")) {
		Logger::critical() << "Could not find required map JSON attributes for" << name;
		return;
	}

	mWidth = doc["width"].asInt();
	mHeight = doc["height"].asInt();

	Json::Value layers = doc["layers"];
	for(uint32_t i = 0; i < layers.size(); i++) {
		Json::Value layerObj = layers[i];
		if(!layerObj.isMember("tileset")) {
			Logger::critical() << "Could not find required map layer JSON attributes for" << name;
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
			Logger::critical() << "Could not find tileset for" << name;
			return;
		}

		if(!layerObj.isMember("data")) {
			Logger::critical() << "Could not find required layer JSON attributes for" << name;
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

		mLayers.push_back(std::shared_ptr<MapLayer>(layer));
	}

	mCollisionMap = std::shared_ptr<CollisionMap>(new CollisionMap(pixelWidth(), pixelHeight()));
	mCollisionMap->set(144, 0, true);
	for(auto it = mLayers.begin(); it != mLayers.end(); it++)
	{
		if(!(*it)->updateCollisionMap(*mCollisionMap))
		{
			return;
		}
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

void Map::checkCollision(const Types::PointF& pos, const Types::Dimension& size, Types::PointF& dst, Types::PointF& velocity) const
{
	if(pos.x + dst.x < 0.0f)
	{
		dst.x = 0.0f;
		velocity.x = 0.0f;
	}

	if(pos.y + dst.y < -(size.height / 2))
	{
		dst.y = 0.0f;
		velocity.y = 0.0f;
	}

	// No need to continue checking.
	if(dst.x == 0.0f && dst.y == 0.0f)
	{
		return;
	}

	Types::Pair diff;
	int8_t rDiff = 0;
	if(dst.x != 0.0f && isColliding({pos.x + dst.x, pos.y}, size, diff, rDiff, false)) {
		if(rDiff == 0) {
			velocity.x = 0.0f;
		}

		if(dst.y == 0.0f) {
			if(rDiff < 0) {
				dst.y = (dst.x < 0.0f) ? dst.x : -dst.x;
			} else if(rDiff > 0) {
				dst.y = (dst.x < 0.0f) ? -dst.x : dst.x;
			}
		}

		if(dst.x < 0.0f) {
			dst.x = diff.first;
		} else {
			dst.x = diff.second;
		}
	}
	if(dst.y != 0.0f && isColliding({pos.x, pos.y + dst.y}, size, diff, rDiff, true)) {
		if(rDiff == 0) {
			velocity.y = 0.0f;
		}

		if(dst.x == 0.0f) {
			if(rDiff < 0) {
				dst.x = (dst.y < 0.0f) ? dst.y : -dst.y;
			} else if(rDiff > 0) {
				dst.x = (dst.y < 0.0f) ? -dst.y : dst.y;
			}
		}

		if(dst.y < 0.0f) {
			dst.y = diff.first;
		} else {
			dst.y = diff.second;
		}
	}
}

bool Map::isColliding(const Types::PointF& pos, const Types::Dimension& size, Types::Pair& diff, int8_t& rDiff, bool vertical) const
{
	rDiff = 0;

	int startY = (size.height / 2);
	Types::Quad foundDiff;
	bool found = mCollisionMap->get(pos.x, pos.y + startY, size.width, startY, &foundDiff);

	// Check if we can move around the obstacle.
	if(vertical) {
		diff.first = foundDiff.y1;
		diff.second = foundDiff.y2;

		int obsdiff = size.width / 2;
		if(	(foundDiff.y1 == 0 && foundDiff.x1 >= 0 && size.width - foundDiff.x2 < obsdiff) ||
			(foundDiff.y2 == 0 && foundDiff.x1 >= 0 && size.width - foundDiff.x2 < obsdiff))
		{
			rDiff = 1;
		} else if(	(foundDiff.y1 == 0 && foundDiff.x2 >= 0 && size.width - foundDiff.x1 < obsdiff) ||
					(foundDiff.y2 == 0 && foundDiff.x2 >= 0 && size.width - foundDiff.x1 < obsdiff))
		{
			rDiff = -1;
		}
	} else {
		diff.first = foundDiff.x1;
		diff.second = foundDiff.x2;

		int obsdiff = size.height / 4;
		if(	(foundDiff.x1 == 0 && foundDiff.y1 >= 0 && startY - foundDiff.y2 < obsdiff) ||
			(foundDiff.x2 == 0 && foundDiff.y1 >= 0 && startY - foundDiff.y2 < obsdiff))
		{
			rDiff = 1;
		} else if(	(foundDiff.x1 == 0 && foundDiff.y2 >= 0 && startY - foundDiff.y1 < obsdiff) ||
					(foundDiff.x2 == 0 && foundDiff.y2 >= 0 && startY - foundDiff.y1 < obsdiff))
		{
			rDiff = -1;
		}
	}

	return found;
}

Types::Dimension Map::getTileDimension() const {
	for(auto it = mTilesets.begin(); it != mTilesets.end(); it++) {
		return it->second->getTileDimension();
	}

	return Types::Dimension(32, 32);
}