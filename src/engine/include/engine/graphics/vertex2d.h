#pragma once

#include <cmath>
#include <cstdint>

#include <array>

namespace engine {
namespace graphics {

class Vector2D;

class Vertex2D {
public:
    static constexpr inline uint32_t Size() { return sizeof(float_t) * 2 * 4; }

    Vertex2D() = default;
    virtual ~Vertex2D() = default;

    const float_t* constData() const;

    Vertex2D& tl(float_t x, float_t y);
    Vertex2D& bl(float_t x, float_t y);
    Vertex2D& tr(float_t x, float_t y);
    Vertex2D& br(float_t x, float_t y);

private:
    std::array<std::array<float_t, 2>, 4> mData;

};
}
}
