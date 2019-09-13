#include <engine/graphics/buffer.h>
#include <engine/graphics/bufferwriter.h> 
#include <engine/graphics/matrix.h>
#include <engine/graphics/quad.h>
#include <engine/graphics/transform.h>
#include <engine/graphics/vector.h>

using namespace engine::graphics;

BufferWriter::BufferWriter(Buffer& buffer, uint32_t offset)
    : mBuffer(buffer)
    , mOffset(offset)
{
    mBuffer.bind();
}

void BufferWriter::skip(uint32_t offset)
{
    mOffset += offset;
}

void BufferWriter::release()
{
    mBuffer.release();
}

BufferWriter& BufferWriter::operator+=(const Color& color)
{
    mOffset += mBuffer.write(mOffset, color);
    return *this;
}

BufferWriter& BufferWriter::operator+=(const Matrix& matrix)
{
    mOffset += mBuffer.write(mOffset, matrix);
    return *this;
}

BufferWriter& BufferWriter::operator+=(const Transform& transform)
{
    mOffset += mBuffer.write(mOffset, transform);
    return *this;
}

BufferWriter& BufferWriter::operator+=(const Vector2D& vector)
{
    mOffset += mBuffer.write(mOffset, vector);
    return *this;
}

BufferWriter& BufferWriter::operator+=(const Vector3D& vector)
{
    mOffset += mBuffer.write(mOffset, vector);
    return *this;
}

BufferWriter& BufferWriter::operator+=(const Vector4D& vector)
{
    mOffset += mBuffer.write(mOffset, vector);
    return *this;
}

BufferWriter& BufferWriter::operator+=(const Quad<2>& quad)
{
    mOffset += mBuffer.write(mOffset, quad);
    return *this;
}

BufferWriter& BufferWriter::operator+=(float_t val)
{
    mOffset += mBuffer.write(mOffset, val);
    return *this;
}

BufferWriter& BufferWriter::operator+=(uint32_t val)
{
    mOffset += mBuffer.write(mOffset, val);
    return *this;
}
