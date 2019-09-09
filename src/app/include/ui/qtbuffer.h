#pragma once

#include <cmath>

#include <QOpenGLBuffer>

#include <engine/graphics/buffer.h>

class QtBuffer : public engine::graphics::Buffer
{
public:
    QtBuffer(uint32_t size = 0);
    virtual ~QtBuffer();

    virtual void bind() override;
    virtual void release() override;

    virtual void resize(uint32_t size) override;

    virtual uint32_t write(uint32_t offset, const engine::graphics::Color& color) override;
    virtual uint32_t write(uint32_t offset, const engine::graphics::Matrix& matrix) override;
    virtual uint32_t write(uint32_t offset, const engine::graphics::Transform& transform) override;
    virtual uint32_t write(uint32_t offset, const engine::graphics::Vector2D& vector) override;
    virtual uint32_t write(uint32_t offset, const engine::graphics::Vertex2D& vertex) override;

    virtual uint32_t write(uint32_t offset, float_t val) override;
    virtual uint32_t write(uint32_t offset, uint32_t val) override;

private:
    QOpenGLBuffer mBuffer;
};
