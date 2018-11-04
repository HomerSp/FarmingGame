#pragma once

#include <memory>
#include <string>

#include <json/json.h>

#include <engine/collisionmap.h>
#include <engine/image.h>

namespace engine {
	class AssetManager {
	public:
		enum Type
		{
			Character = 0,
			Map,
			Charset,
			Tileset,
		};

		static std::shared_ptr<Json::Value> data(Type type, const std::string& name);
		static std::shared_ptr<engine::Image> image(Type type, const std::string& name);
		static std::shared_ptr<engine::CollisionMap> collision(Type type, const std::string& name);
		
		static std::string dataPath(Type type, const std::string& name);
		static std::string imagePath(Type type, const std::string& name);
		static std::string collisionPath(Type type, const std::string& name);

	private:
		static std::string sBase;
	};
}
