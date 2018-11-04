#include <fstream>

#include <engine/logger.h>
#include <engine/assetmanager.h>

using namespace engine;

std::string AssetManager::sBase = "assets";

std::shared_ptr<Json::Value> AssetManager::data(Type type, const std::string& name)
{
	std::shared_ptr<Json::Value> doc = std::shared_ptr<Json::Value>(new Json::Value());

	std::string path = AssetManager::dataPath(type, name);
	if(path.length() > 0)
	{
		std::ifstream file(path);
		file >> *doc.get();
	}

	return doc;
}

std::shared_ptr<engine::Image> AssetManager::image(Type type, const std::string& name)
{
	std::string path = AssetManager::imagePath(type, name);
	if(path.length() > 0) {
		return std::shared_ptr<engine::Image>(new engine::Image(path));
	}

	return std::shared_ptr<engine::Image>(new engine::Image());
}

std::shared_ptr<engine::CollisionMap> AssetManager::collision(Type type, const std::string& name)
{
	std::string path = AssetManager::collisionPath(type, name);
	if(path.length() > 0) {
		return std::shared_ptr<engine::CollisionMap>(new engine::CollisionMap(path));
	}

	return std::shared_ptr<engine::CollisionMap>(new engine::CollisionMap());
}

std::string AssetManager::dataPath(Type type, const std::string& name)
{
	std::string ret = AssetManager::sBase + "/data/";
	switch(type) {
	case Character:
		ret += "character";
		break;
	case Map:
		ret += "map";
		break;
	case Charset:
		ret += "charset";
		break;
	case Tileset:
		ret += "tileset";
		break;
	default:
		return "";
	}
	return ret + "/" + name + ".json";
}

std::string AssetManager::imagePath(Type type, const std::string& name)
{
	std::string ret = AssetManager::sBase + "/image/";
	switch(type) {
	case Charset:
		ret += "charset";
		break;
	case Tileset:
		ret += "tileset";
		break;
	default:
		return "";
	}
	return ret + "/" + name + ".png";
}

std::string AssetManager::collisionPath(Type type, const std::string& name)
{
	std::string ret = AssetManager::sBase + "/collision/";
	switch(type) {
	case Tileset:
		ret += "tileset";
		break;
	default:
		return "";
	}
	return ret + "/" + name + ".png";
}
