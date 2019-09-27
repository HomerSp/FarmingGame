#include <engine/graphics/buffer.h>
#include <engine/graphics/matrix.h>
#include <engine/graphics/quad.h>
#include <engine/graphics/vector.h>

using namespace engine::graphics;

Buffer::Writer::Writer(Buffer& buffer, uint32_t offset)
    : mBuffer(buffer)
    , mOffset(offset)
{
    mBuffer.bind();
}

void Buffer::Writer::skip(uint32_t offset)
{
    mOffset += offset;
}

void Buffer::Writer::release()
{
    mBuffer.release();
}

Buffer::Writer& Buffer::Writer::append(const Color& color)
{
    mOffset += mBuffer.write(mOffset, color);
    return *this;
}

Buffer::Writer& Buffer::Writer::append(const Matrix& matrix)
{
    mOffset += mBuffer.write(mOffset, matrix);
    return *this;
}

Buffer::Writer& Buffer::Writer::append(const Vector2D& vector)
{
    mOffset += mBuffer.write(mOffset, vector);
    return *this;
}

Buffer::Writer& Buffer::Writer::append(const Vector3D& vector)
{
    mOffset += mBuffer.write(mOffset, vector);
    return *this;
}

Buffer::Writer& Buffer::Writer::append(const Vector4D& vector)
{
    mOffset += mBuffer.write(mOffset, vector);
    return *this;
}

Buffer::Writer& Buffer::Writer::append(const Quad<2>& quad)
{
    mOffset += mBuffer.write(mOffset, quad);
    return *this;
}

Buffer::Writer& Buffer::Writer::append(int val)
{
    mOffset += mBuffer.write(mOffset, val);
    return *this;
}

Buffer::Writer& Buffer::Writer::append(float_t val)
{
    mOffset += mBuffer.write(mOffset, val);
    return *this;
}

Buffer::Writer& Buffer::Writer::append(uint32_t val)
{
    mOffset += mBuffer.write(mOffset, val);
    return *this;
}

Buffer::Writer Buffer::writer(uint32_t offset)
{
    return {*this, offset};
}