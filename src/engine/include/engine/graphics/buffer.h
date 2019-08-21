#pragma once

#include <cmath>
#include <cstdint>

#include <engine/graphics/color.h>
#include <engine/graphics/matrix.h>
#include <engine/graphics/vertex.h>
#include <engine/types.h>

namespace engine {
namespace graphics {
class Buffer {
public:
    Buffer() = default;
    virtual ~Buffer() = default;

    virtual void bind() = 0;
    virtual void release() = 0;

    virtual void resize(uint32_t size) = 0;

    virtual uint32_t write(uint32_t offset, const Matrix& matrix) = 0;
    virtual uint32_t write(uint32_t offset, const Color& color) = 0;
    virtual uint32_t write(uint32_t offset, const Vertex2D& vertex) = 0;

private:
};
}
}
