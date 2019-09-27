#pragma once

#include <cstdint>

#include <engine/graphics/quad.h>
#include <engine/types.h>

namespace engine {
namespace graphics {

class Color;
class Matrix;
class Vector2D;
class Vector3D;
class Vector4D;

class Buffer {
public:
    class Writer {
    public:
        Writer(Buffer& buffer, uint32_t offset = 0);

        void skip(uint32_t offset);

        void release();

        Writer& append(const Color& color);
        Writer& append(const Matrix& matrix);
        Writer& append(const Vector2D& vector);
        Writer& append(const Vector3D& vector);
        Writer& append(const Vector4D& vector);
        Writer& append(const Quad<2>& quad);

        Writer& append(int val);
        Writer& append(float_t val);
        Writer& append(uint32_t val);

    private:
        Buffer& mBuffer;
        uint32_t mOffset;
    };
public:
    Buffer() = default;
    virtual ~Buffer() = default;

    Writer writer(uint32_t offset = 0);

    virtual void bind() = 0;
    virtual void release() = 0;

    virtual void resize(uint32_t size) = 0;

    virtual uint32_t write(uint32_t offset, const Color& color) = 0;
    virtual uint32_t write(uint32_t offset, const Matrix& matrix) = 0;
    virtual uint32_t write(uint32_t offset, const Vector2D& vector) = 0;
    virtual uint32_t write(uint32_t offset, const Vector3D& vector) = 0;
    virtual uint32_t write(uint32_t offset, const Vector4D& vector) = 0;
    virtual uint32_t write(uint32_t offset, const Quad<2>& quad) = 0;

    virtual uint32_t write(uint32_t offset, int val) = 0;
    virtual uint32_t write(uint32_t offset, float_t val) = 0;
    virtual uint32_t write(uint32_t offset, uint32_t val) = 0;

private:
};
}
}
