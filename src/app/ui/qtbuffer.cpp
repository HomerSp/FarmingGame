#include <QMatrix4x4>

#include <ui/qtbuffer.h>
#include <ui/qttransform.h>

#include <engine/logger.h>

#include <string>

QtBuffer::QtBuffer(uint32_t size)
{
    mBuffer.create();
    if (size > 0) {
        mBuffer.bind();
        mBuffer.allocate(size);
        mBuffer.release();
    }
}

QtBuffer::~QtBuffer()
{
    mBuffer.destroy();
}

void QtBuffer::bind()
{
    mBuffer.bind();
}

void QtBuffer::release()
{
    mBuffer.release();
}

void QtBuffer::resize(uint32_t size)
{
    if (size == 0) {
        return;
    }

    mBuffer.destroy();
    mBuffer.create();
    mBuffer.bind();
    mBuffer.allocate(size);
    mBuffer.release();
}

uint32_t QtBuffer::write(uint32_t offset, const engine::graphics::Color& color)
{
    mBuffer.write(offset, color.constData(), engine::graphics::Color::Size);
    return engine::graphics::Color::Size;
}

uint32_t QtBuffer::write(uint32_t offset, const engine::graphics::Matrix& matrix)
{
    mBuffer.write(offset, matrix.constData(), engine::graphics::Matrix::Size);
    return engine::graphics::Matrix::Size;
}

uint32_t QtBuffer::write(uint32_t offset, const engine::graphics::Transform& transform)
{
    const auto& native = dynamic_cast<const QtTransform&>(transform);
    auto matrix = static_cast<QMatrix4x4>(native);
    mBuffer.write(offset, matrix.constData(), engine::graphics::Matrix::Size);
    return engine::graphics::Matrix::Size;
}

uint32_t QtBuffer::write(uint32_t offset, const engine::graphics::Vertex2D& vertex)
{
    mBuffer.write(offset, vertex.constData(), engine::graphics::Vertex2D::Size);
    return engine::graphics::Vertex2D::Size;
}
