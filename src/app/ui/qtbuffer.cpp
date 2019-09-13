#include <QMatrix4x4>

#include <engine/graphics/matrix.h>
#include <engine/graphics/quad.h>
#include <engine/graphics/vector.h>

#include <ui/qtbuffer.h>
#include <ui/qttransform.h>

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
    mBuffer.write(offset, color.constData(), engine::graphics::Color::Size());
    return engine::graphics::Color::Size();
}

uint32_t QtBuffer::write(uint32_t offset, const engine::graphics::Matrix& matrix)
{
    mBuffer.write(offset, matrix.constData(), engine::graphics::Matrix::Size());
    return engine::graphics::Matrix::Size();
}

uint32_t QtBuffer::write(uint32_t offset, const engine::graphics::Transform& transform)
{
    const auto& native = dynamic_cast<const QtTransform&>(transform);
    auto matrix = static_cast<QMatrix4x4>(native);
    mBuffer.write(offset, matrix.constData(), engine::graphics::Matrix::Size());
    return engine::graphics::Matrix::Size();
}

uint32_t QtBuffer::write(uint32_t offset, const engine::graphics::Vector2D& vector)
{
    mBuffer.write(offset, vector.constData(), engine::graphics::Vector2D::Size());
    return engine::graphics::Vector2D::Size();
}

uint32_t QtBuffer::write(uint32_t offset, const engine::graphics::Vector3D& vector)
{
    mBuffer.write(offset, vector.constData(), engine::graphics::Vector3D::Size());
    return engine::graphics::Vector3D::Size();
}

uint32_t QtBuffer::write(uint32_t offset, const engine::graphics::Vector4D& vector)
{
    mBuffer.write(offset, vector.constData(), engine::graphics::Vector4D::Size());
    return engine::graphics::Vector4D::Size();
}

uint32_t QtBuffer::write(uint32_t offset, const engine::graphics::Quad<2>& quad)
{
    mBuffer.write(offset, quad.constData(), engine::graphics::Quad<2>::Size());
    return engine::graphics::Quad<2>::Size();
}

uint32_t QtBuffer::write(uint32_t offset, float_t val)
{
    mBuffer.write(offset, &val, sizeof(float_t));
    return sizeof(float_t);
}

uint32_t QtBuffer::write(uint32_t offset, uint32_t val)
{
    mBuffer.write(offset, &val, sizeof(uint32_t));
    return sizeof(uint32_t);
}
