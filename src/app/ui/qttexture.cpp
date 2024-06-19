#include <QImage>
#include <QOpenGLPixelTransferOptions>
#include <QString>

#include <ui/qttexture.h>

QtTexture::QtTexture(uint32_t w, uint32_t h, uint32_t layers)
    : engine::graphics::Texture(w, h, layers)
{
    mTexture = std::make_unique<QOpenGLTexture>((layers > 0) ? QOpenGLTexture::Target2DArray : QOpenGLTexture::Target2D);
    mTexture->create();
    if (layers > 0) {
        mTexture->setLayers(layers);
    }
    mTexture->setFormat(QOpenGLTexture::RGBA8_UNorm);
    mTexture->setSize(w, h);
    mTexture->allocateStorage(QOpenGLTexture::RGBA, QOpenGLTexture::UInt8);
    mTexture->setMinificationFilter(QOpenGLTexture::LinearMipMapLinear);
    mTexture->setMagnificationFilter(QOpenGLTexture::Linear);
}

QtTexture::QtTexture(const QImage& img)
    : QtTexture(img.width(), img.height(), 1)
{
    auto copied = img.copy({0, 0, mTexture->width(), mTexture->height()});
    QOpenGLPixelTransferOptions uploadOptions;
    uploadOptions.setAlignment(1);
    mTexture->setData(0, 0, QOpenGLTexture::RGBA, QOpenGLTexture::UInt8, copied.constBits(), &uploadOptions);

    setDimensions(engine::Types::Dimension<>(img.width(), img.height()), 0);
}

void QtTexture::bind(uint32_t id)
{
    mTexture->bind(id);
}

void QtTexture::release()
{
    mTexture->release();
}

void QtTexture::setData(const engine::graphics::Image& image, uint32_t layer)
{
    auto copied = image.copy(engine::Types::Rect<uint32_t>(0, 0, mTexture->width(), mTexture->height()));
    QOpenGLPixelTransferOptions uploadOptions;
    uploadOptions.setAlignment(1);
    mTexture->setData(0, layer, QOpenGLTexture::RGBA, QOpenGLTexture::UInt8, copied.data(), &uploadOptions);

    setDimensions(engine::Types::Dimension<>(image.width(), image.height()), layer);
}

uint32_t QtTexture::width() const
{
    return mTexture->width();
}

uint32_t QtTexture::height() const
{
    return mTexture->height();
} 
