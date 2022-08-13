#pragma once

#include <vector>

#include <engine/types.h>

namespace engine {
class CollisionMap {
public:
    CollisionMap(uint32_t width = 0, uint32_t height = 0);
    CollisionMap(const std::string& path);

    std::pair<bool, bool> check(const Types::Point<uint32_t>& pos, const Types::Point<>& dst, const Types::Dimension<>& size, Types::Quad<>* diff = nullptr) const;

    bool solid(uint32_t x, uint32_t y) const { return bounds(x, y) && mSolid[x + y * mWidth]; }
    bool solid(const Types::Point<uint32_t>& pos, const Types::Dimension<>& size) const;
    bool transparent(uint32_t x, uint32_t y, uint32_t width = 1, uint32_t height = 1) const;

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

    bool checkX(const Types::Point<uint32_t>& pos, int32_t dst, const Types::Dimension<>& size, Types::Quad<>* diff) const;
    bool checkY(const Types::Point<uint32_t>& pos, int32_t dst, const Types::Dimension<>& size, Types::Quad<>* diff) const;

private:
    bool mValid;
    uint32_t mWidth;
    uint32_t mHeight;
    std::vector<bool> mSolid;
    std::vector<bool> mTransparent;
};
}