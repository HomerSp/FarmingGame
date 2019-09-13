#pragma once

#include <cmath>
#include <cstdint>

namespace engine {
namespace graphics {
class Matrix {
public:
    static constexpr inline uint32_t Size() { return sizeof(float_t) * 4 * 4; };

    Matrix() = default;
    virtual ~Matrix() = default;

    virtual const float_t* constData() const = 0;

    virtual void reset() = 0;

    virtual void frustum(float_t left, float_t right, float_t top, float_t bottom, float_t near, float_t far) = 0;
    virtual void ortho(float_t left, float_t right, float_t top, float_t bottom, float_t near, float_t far) = 0;
    virtual void viewport(float_t left, float_t right, float_t top, float_t bottom, float_t near, float_t far) = 0;

    virtual void rotate(float_t angle, float_t z) = 0;

    virtual void scale(float_t x, float_t y) = 0;
    virtual void scale(float_t x, float_t y, float_t z) = 0;

    virtual void translate(float_t x, float_t y) = 0;
    virtual void translate(float_t x, float_t y, float_t z) = 0;

    virtual Matrix& operator*=(const Matrix& o) = 0;

private:
};
}
}
