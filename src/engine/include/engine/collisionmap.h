#pragma once

#include <map>
#include <string>

namespace engine
{
	class CollisionMap
	{
	public:
		CollisionMap(const std::string& name);

	private:
		std::map<int, bool> mSolid;
	};
}