#pragma once

#include <cstdint>
#include <memory>

#include <QOpenGLTexture>

#include <engine/graphics/image.h>
#include <engine/graphics/texture.h>

class QtTexture : public engine::graphics::Texture {
public:
    QtTexture(uint32_t w, uint32_t h, uint32_t layers);
    virtual ~QtTexture() = default;

    virtual void bind(uint32_t id) override;
    virtual void release() override;

    virtual void setData(const engine::graphics::Image& image, uint32_t layer) override;

    virtual uint32_t width() const override;
    virtual uint32_t height() const override;

private:
    std::unique_ptr<QOpenGLTexture> mTexture;
}; 
