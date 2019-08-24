#pragma once

#include <cmath>
#include <cstdint>

#include <engine/graphics/buffer.h>
#include <engine/graphics/matrix.h>
#include <engine/graphics/transform.h>
#include <engine/graphics/vertex.h>
#include <engine/types.h>

namespace engine {
namespace graphics {
class BufferWriter {
public:
    BufferWriter(Buffer& buffer, uint32_t offset = 0);

    void release();

    BufferWriter& operator+=(const Color& color);
    BufferWriter& operator+=(const Matrix& matrix);
    BufferWriter& operator+=(const Transform& transform);
    BufferWriter& operator+=(const Vertex2D& vertex);

private:
    Buffer& mBuffer;
    uint32_t mOffset;
};
}
}
