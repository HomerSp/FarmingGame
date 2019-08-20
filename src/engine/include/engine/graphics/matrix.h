#pragma once

#include <cmath>
#include <cstdint>

namespace engine {
namespace graphics {
class Matrix {
public:
    static constexpr uint32_t Size = sizeof(float_t) * 4 * 4;

    Matrix() = default;
    virtual ~Matrix() = default;

    virtual const float_t* constData() const = 0;

    virtual uint32_t size() = 0;

    virtual void reset() = 0;

    virtual void ortho(float_t left, float_t right, float_t top, float_t bottom, float_t near, float_t far) = 0;

    virtual void scale(float_t x, float_t y) = 0;
    virtual void scale(float_t x, float_t y, float_t z) = 0;

    virtual void translate(float_t x, float_t y) = 0;
    virtual void translate(float_t x, float_t y, float_t z) = 0;

private:
};
}
}
