#pragma once

#include <vector>

#include <engine/types.h>

namespace engine {
class CollisionMap {
public:
    CollisionMap(uint32_t width = 0, uint32_t height = 0);
    CollisionMap(const std::string& path);

    bool get(uint32_t x, uint32_t y, uint32_t width = 1, uint32_t height = 1, Types::Quad<>* diff = nullptr) const;

    void set(uint32_t x, uint32_t y, bool b);

#ifdef DEBUG
    void save(const std::string& path);
#endif

    bool operator!()
    {
        return !mValid;
    }

protected:
    inline bool bounds(uint32_t x, uint32_t y) const
    {
        return (x + (y * mWidth) < mSolid.size());
    }

private:
    bool mValid;
    uint32_t mWidth;
    uint32_t mHeight;
    std::vector<bool> mSolid;
};
}