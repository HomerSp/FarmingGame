#pragma once

#include <memory>
#include <string>

#include <json/json.h>

#include <engine/collisionmap.h>
#include <engine/image.h>

namespace engine
{
	class AssetManager
	{
	public:
		enum Type
		{
			Character = 0,
			Map,
			Charset,
			Tileset,
		};

		static std::shared_ptr<AssetManager> get();

		std::shared_ptr<Json::Value> data(Type type, const std::string& name);
		std::shared_ptr<engine::Image> image(Type type, const std::string& name);
		std::shared_ptr<engine::CollisionMap> collision(Type type, const std::string& name);

		std::string dataPath(Type type, const std::string& name);
		std::string imagePath(Type type, const std::string& name);
		std::string collisionPath(Type type, const std::string& name);

	protected:
		AssetManager();

	private:
		static std::shared_ptr<AssetManager> sInstance;

		std::string mBase;
	};
}
