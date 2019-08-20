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

    virtual uint32_t write(uint32_t offset, const engine::graphics::Matrix& matrix) override;
    virtual uint32_t write(uint32_t offset, const engine::graphics::Color& color) override;
    virtual uint32_t write(uint32_t offset, const engine::graphics::Vertex2D& vertex) override;

private:
    QOpenGLBuffer mBuffer;
};
