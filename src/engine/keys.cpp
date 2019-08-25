#include <engine/keys.h>

using namespace engine;

KeyItem::KeyItem(Keys::Type key)
	: key(key)
	, start(std::chrono::steady_clock::now())
	, down(true)
	, longPress(false)
{
}

void KeyList::setDown(int32_t key)
{
    if (mKeyTable.find(key) == mKeyTable.end()) {
        return;
    }

    auto type = mKeyTable.find(key)->second;
    if (!empty() && back().key == type) {
        return;
    }

    remove_if([type](KeyItem n) { return n.key == type; });
    push_back({type});
}

void KeyList::setUp(int32_t key)
{
    if (mKeyTable.find(key) == mKeyTable.end()) {
        return;
    }

    Keys::Type type = mKeyTable.find(key)->second;
    for (auto& item: *this) {
    	if (item.key == type) {
    		item.down = false;
    	}
    }
}

bool KeyList::down(Keys::Type key)
{
	for (const auto& i: *this) {
		if (i.key == key) {
			return i.down;
		}
	}

    return false;
}

bool KeyList::up(Keys::Type key)
{
	for (const auto& i: *this) {
		if (i.key == key) {
			return !i.longPress && !i.down;
		}
	}

    return false;
}

bool KeyList::longPress(Keys::Type key)
{
	for (const auto& i: *this) {
		if (i.key == key) {
			return i.longPress;
		}
	}

    return false;
}

void KeyList::update()
{
    remove_if([](KeyItem n) { return !n.down; });

    auto now = std::chrono::steady_clock::now();
    for (auto& item: *this) {
    	if (item.longPress) {
    		continue;
    	}

    	uint64_t d = std::chrono::duration_cast<std::chrono::milliseconds>(now - item.start).count();
    	if (d >= 250) {
    		item.longPress = true;
    	}
    }
}


void KeyList::mapKey(int32_t i, Keys::Type key)
{
	mKeyTable[i] = key;
}
