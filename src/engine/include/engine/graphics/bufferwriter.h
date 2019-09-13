#pragma once

#include <cstdint>

#include <engine/graphics/quad.h>
#include <engine/types.h>

namespace engine {
namespace graphics {

class Buffer;
class Color;
class Matrix;
class Transform;
class Vector2D;
class Vector3D;
class Vector4D;

class BufferWriter {
public:
    BufferWriter(Buffer& buffer, uint32_t offset = 0);

    void skip(uint32_t offset);

    void release();

    BufferWriter& operator+=(const Color& color);
    BufferWriter& operator+=(const Matrix& matrix);
    BufferWriter& operator+=(const Transform& transform);
    BufferWriter& operator+=(const Vector2D& vector);
    BufferWriter& operator+=(const Vector3D& vector);
    BufferWriter& operator+=(const Vector4D& vector);
    BufferWriter& operator+=(const Quad<2>& quad);

    BufferWriter& operator+=(float_t val);
    BufferWriter& operator+=(uint32_t val);

private:
    Buffer& mBuffer;
    uint32_t mOffset;
};
}
}
