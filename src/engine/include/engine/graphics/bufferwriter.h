#pragma once

#include <cstdint>

#include <engine/types.h>

namespace engine {
namespace graphics {

class Buffer;
class Color;
class Matrix;
class Transform;
class Vector2D;
class Vertex2D;

class BufferWriter {
public:
    BufferWriter(Buffer& buffer, uint32_t offset = 0);

    void release();

    BufferWriter& operator+=(const Color& color);
    BufferWriter& operator+=(const Matrix& matrix);
    BufferWriter& operator+=(const Transform& transform);
    BufferWriter& operator+=(const Vector2D& vector);
    BufferWriter& operator+=(const Vertex2D& vertex);

    BufferWriter& operator+=(float_t val);
    BufferWriter& operator+=(uint32_t val);

private:
    Buffer& mBuffer;
    uint32_t mOffset;
};
}
}
