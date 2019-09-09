#pragma once

#include <cstdint>

#include <engine/types.h>

namespace engine {
namespace graphics {

class Color;
class Matrix;
class Transform;
class Vector2D;
class Vertex2D;

class Buffer {
public:
    Buffer() = default;
    virtual ~Buffer() = default;

    virtual void bind() = 0;
    virtual void release() = 0;

    virtual void resize(uint32_t size) = 0;

    virtual uint32_t write(uint32_t offset, const Color& color) = 0;
    virtual uint32_t write(uint32_t offset, const Matrix& matrix) = 0;
    virtual uint32_t write(uint32_t offset, const Transform& transform) = 0;
    virtual uint32_t write(uint32_t offset, const Vector2D& vector) = 0;
    virtual uint32_t write(uint32_t offset, const Vertex2D& vertex) = 0;

    virtual uint32_t write(uint32_t offset, float_t val) = 0;
    virtual uint32_t write(uint32_t offset, uint32_t val) = 0;

private:
};
}
}
