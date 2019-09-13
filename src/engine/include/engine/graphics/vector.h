#pragma once

#include <cmath>
#include <cstdint>

#include <array>

namespace engine {
namespace graphics {

template<size_t N>
class Vector {
public:
    static constexpr inline uint32_t Size() { return sizeof(float_t) * N; }

    Vector()
        : mData({})
    {
    }

    virtual ~Vector() = default;

    const float_t* constData() const
    {
        return &mData[0];
    }

    const float_t& operator[](uint32_t index) const
    {
        return mData.at(index);
    }

    float_t& operator[](uint32_t index)
    {
        return mData.at(index);
    }

private:
    std::array<float_t, N> mData;

};

class Vector2D : public Vector<2> {
public:
    static constexpr inline uint32_t Size() { return Vector<2>::Size(); }

    Vector2D(float_t a = 0.0f, float_t b = 0.0f)
        : Vector<2>()
    {
        (*this)[0] = a;
        (*this)[1] = b;
    }
};

class Vector3D : public Vector<3> {
public:
    static constexpr inline uint32_t Size() { return Vector<3>::Size(); }

    Vector3D(float_t a = 0.0f, float_t b = 0.0f, float_t c = 0.0f)
        : Vector<3>()
    {
        (*this)[0] = a;
        (*this)[1] = b;
        (*this)[2] = c;
    }
};

class Vector4D : public Vector<4> {
public:
    static constexpr inline uint32_t Size() { return Vector<4>::Size(); }

    Vector4D(float_t a = 0.0f, float_t b = 0.0f, float_t c = 0.0f, float_t d = 0.0f)
        : Vector<4>()
    {
        (*this)[0] = a;
        (*this)[1] = b;
        (*this)[2] = c;
        (*this)[3] = d;
    }
};
}
}
