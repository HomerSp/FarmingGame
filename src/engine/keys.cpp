#include <algorithm>

#include <engine/keys.h>

using namespace engine;

void KeyList::append(Keys::Type key)
{
	if (!empty() && back() == key)
	{
		return;
	}

	remove_if([key](Keys::Type n){ return n == key; });
	push_back(key);
}

void KeyList::append(int key)
{
	if (mKeyTable.find(key) == mKeyTable.end())
	{
		return;
	}

	append(mKeyTable.find(key)->second);
}

void KeyList::remove(int key)
{
	if (mKeyTable.find(key) == mKeyTable.end())
	{
		return;
	}

	Keys::Type type = mKeyTable.find(key)->second;
	remove_if([type](Keys::Type n){ return n == type; });
}

bool KeyList::contains(Keys::Type key)
{
	return std::find(begin(), end(), key) != end();
}