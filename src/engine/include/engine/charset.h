#pragma once

#include <map>
#include <memory>

#include <json/json.h>

#include <engine/types.h>
#include <engine/renderer.h>
#include <engine/image.h>

namespace engine {
	struct CharsetNode {
	public:
		CharsetNode(const Types::Rect& rc, const Types::Cells& cells);

		Types::Rect rect;
		Types::Cells cells;
	};

	class Charset {
	public:
		enum Type {
			TypeWalk = 0,
		};

		Charset(const std::string& name);

		void draw(Renderer& renderer, const Types::Point& pos, Charset::Type type, int direction = 0, int frame = 0);

		int width(Charset::Type type);
		int height(Charset::Type type);

		int columns(Charset::Type type);

		bool operator!() const {
			return !mValid;
		}

	protected:
		void addNode(Charset::Type type, Json::Value& val);

	private:
		bool mValid;
		std::shared_ptr<engine::Image> mImage;
		std::map<Charset::Type, std::shared_ptr<CharsetNode> > mNodes;

	};
} 
