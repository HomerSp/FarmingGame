#pragma once

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

        // Just for testing
        TestFriction,
        TestSlowMode,
        TestFastForward,
        TestPause,
    } Type;
};

class KeyList : public std::list<Keys::Type> {
public:
    explicit KeyList() = default;

    void append(Keys::Type key);
    void append(int32_t key);

    void remove(int32_t key);

    bool contains(Keys::Type key);

    Keys::Type& operator[](int32_t type)
    {
        return mKeyTable[type];
    }

private:
    std::unordered_map<int32_t, Keys::Type> mKeyTable;
};
}
