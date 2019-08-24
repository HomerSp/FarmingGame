#pragma once

#include <cmath>
#include <cstdint>

namespace engine {
namespace graphics {
class Transform {
public:
    Transform() = default;
    virtual ~Transform() = default;

    virtual void reset() = 0;

    virtual void rotate(float_t angle) = 0;
    virtual void rotateOrigin(float_t angle, float_t x, float_t y) = 0;

    virtual void scale(float_t x, float_t y) = 0;
    virtual void translate(float_t x, float_t y) = 0;

    virtual Transform& operator*=(const Transform& o) = 0;

private:
};
}
}
