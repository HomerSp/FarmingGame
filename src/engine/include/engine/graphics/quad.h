#pragma once

#include <cmath>
#include <cstdint>

#include <array>

namespace engine {
namespace graphics {

template<size_t N>
class Quad {
public:
    static constexpr inline uint32_t Size() { return sizeof(float_t) * N * 4; }

    Quad()
        : mData({})
    {
    }

    virtual ~Quad() = default;

    const float_t* constData() const
    {
        return &mData[0][0];
    }

    Quad<N>& lt(float_t x, float_t y)
    {
        mData[0][0] = x;
        mData[0][1] = y;
        return *this;
    }

    Quad<N>& lb(float_t x, float_t y)
    {
        mData[1][0] = x;
        mData[1][1] = y;
        return *this;
    }

    Quad<N>& rt(float_t x, float_t y)
    {
        mData[2][0] = x;
        mData[2][1] = y;
        return *this;
    }

    Quad<N>& rb(float_t x, float_t y)
    {
        mData[3][0] = x;
        mData[3][1] = y;
        return *this;
    }

private:
    std::array<std::array<float_t, N>, 4> mData;

};

class Quad2D : public Quad<2> {
public:
    static constexpr inline uint32_t Size() { return Quad<2>::Size(); }

    Quad2D()
        : Quad<2>()
    {
    }
};
}
}
