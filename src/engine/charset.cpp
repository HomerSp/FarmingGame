#include <json/json.h>

#include <engine/assetmanager.h>
#include <engine/charset.h>
#include <engine/logger.h>

using namespace engine;

CharsetNode::CharsetNode(const Types::Rect& rc, const Types::Cells& cells)
	: rect(rc)
	, cells(cells)
{

}

Charset::Charset(const std::string& name)
	: mValid(false)
	, mImage(nullptr)
{
	std::shared_ptr<Json::Value> docPtr = AssetManager::get()->data(AssetManager::get()->Charset, name);
	Json::Value& doc = *docPtr;
	if (!doc.isObject())
	{
		Logger::critical() << "Could not open charset JSON file" << name;
		return;
	}

	if (!doc.isMember("image"))
	{
		Logger::critical() << "Could not find required charset JSON data for" << name;
		return;
	}

	mImage = AssetManager::get()->image(AssetManager::get()->Charset, doc["image"].asString());
	if (!*mImage)
	{
		Logger::critical() << "Could not load charset image for" << name;
		return;
	}

	if (doc.isMember("walk"))
	{
		addNode(Charset::TypeWalk, doc["walk"]);
	}

	mValid = !mNodes.empty();
}

void Charset::draw(Renderer& renderer, const Types::Point& pos, Charset::Type type, int direction, int frame)
{
	// Couldn't find the node, return...
	if (mNodes.find(type) == mNodes.end())
	{
		return;
	}

	std::shared_ptr<CharsetNode> node = mNodes.find(type)->second;
	Types::Rect dst(pos.x, pos.y, node->rect.width, node->rect.height);
	Types::Rect src(node->rect.x + (node->rect.width * frame), node->rect.y + (node->rect.height * direction), node->rect.width, node->rect.height);
	renderer.drawImage(*mImage, dst, src);
}

int Charset::width(Charset::Type type)
{
	if (mNodes.find(type) == mNodes.end())
	{
		return 0;
	}

	return mNodes.at(type)->rect.width;
}

int Charset::height(Charset::Type type)
{
	if (mNodes.find(type) == mNodes.end())
	{
		return 0;
	}

	return mNodes.at(type)->rect.height;
}

int Charset::columns(Charset::Type type)
{
	if (mNodes.find(type) == mNodes.end())
	{
		return 0;
	}

	return mNodes.find(type)->second->cells.cols;
}

void Charset::addNode(Charset::Type type, Json::Value& val)
{
	if (!val.isMember("size") || !val.isMember("pos") || !val.isMember("cells"))
	{
		return;
	}

	Json::Value size = val["size"];
	Json::Value pos = val["pos"];
	Json::Value cells = val["cells"];
	if (size.size() != 2 || pos.size() != 2 || cells.size() != 2)
	{
		return;
	}

	Types::Rect rect(pos[0].asInt(), pos[1].asInt(), size[0].asInt(), size[1].asInt());
	Types::Cells c(cells[0].asInt(), cells[1].asInt());
	mNodes[type] = std::make_shared<CharsetNode>(rect, c);
}
