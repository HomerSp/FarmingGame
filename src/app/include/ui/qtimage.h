#pragma once

#include <cstdint>

#include <QImage>
#include <QOpenGLTexture>

#include <engine/graphics/image.h>

class QtImage : public engine::graphics::Image {
public:
    QtImage(const std::string& path);
    virtual ~QtImage() = default;

    const QImage& image() const;
    QOpenGLTexture& texture() const;

    virtual uint32_t width() const;
    virtual uint32_t height() const;

private:
    std::unique_ptr<QImage> mImage;
    std::unique_ptr<QOpenGLTexture> mTexture;
}; 
