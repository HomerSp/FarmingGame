#pragma once

#include <chrono>
#include <list>
#include <unordered_map>

namespace engine {
struct Keys {
public:
    typedef enum {
        None = 0,
        Down,
        Left,
        Right,
        Up,
        Run,
        Walk,
        Use,
        ExpandHudItems,

        // Just for testing
        TestFriction,
        TestSlowMode,
        TestFastForward,
        TestPause,
    } Type;
};

struct KeyItem {
public:
	KeyItem(Keys::Type key);

	Keys::Type key;
	std::chrono::time_point<std::chrono::steady_clock> start;
	bool down;
	bool longPress;
};

class KeyList : public std::list<KeyItem> {
public:
    explicit KeyList() = default;

    void setDown(int32_t key);
    void setUp(int32_t key);

    bool down(Keys::Type key);
    bool up(Keys::Type key);
    bool longPress(Keys::Type key);

    void update();

    void mapKey(int32_t i, Keys::Type key);

private:
    std::unordered_map<int32_t, Keys::Type> mKeyTable;
};
}
