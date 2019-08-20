#include <engine/graphics/bufferwriter.h> 

using namespace engine::graphics;

BufferWriter::BufferWriter(Buffer& buffer, uint32_t offset)
    : mBuffer(buffer)
    , mOffset(offset)
{
    mBuffer.bind();
}

void BufferWriter::release()
{
    mBuffer.release();
}

BufferWriter& BufferWriter::operator+=(const Matrix& matrix)
{
    mOffset += mBuffer.write(mOffset, matrix);
    return *this;
}

BufferWriter& BufferWriter::operator+=(const graphics::Color& color)
{
    mOffset += mBuffer.write(mOffset, color);
    return *this;
}

BufferWriter& BufferWriter::operator+=(const Vertex2D& vertex)
{
    mOffset += mBuffer.write(mOffset, vertex);
    return *this;
}
