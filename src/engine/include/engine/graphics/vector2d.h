#pragma once

#include <cmath>
#include <cstdint>

#include <array>

namespace engine {
namespace graphics {

class Vector2D {
public:
    static constexpr inline uint32_t Size() { return sizeof(float_t) * 2; }

    Vector2D(float_t first = 0.0f, float_t second = 0.0f);
    virtual ~Vector2D() = default;

    const float_t* constData() const;

    const float_t& operator[](uint32_t index) const;
    float_t& operator[](uint32_t index);

private:
    std::array<float_t, 2> mData;

};

class Vector4D {
public:
    static constexpr inline uint32_t Size() { return sizeof(float_t) * 4; }

    Vector4D() = default;
    virtual ~Vector4D() = default;

    const float_t* constData() const;

    const float_t& operator[](uint32_t index) const;
    float_t& operator[](uint32_t index);

private:
    std::array<float_t, 4> mData;

};
}
}
