#include <ui/qtimage.h>

QtImage::QtImage(const std::string& path)
{
    mImage = std::make_unique<QImage>(QString(path.c_str()));
    mTexture = std::make_unique<QOpenGLTexture>(*mImage);
}

const QImage& QtImage::image() const
{
    return *mImage;
}

QOpenGLTexture& QtImage::texture() const
{
    return *mTexture;
}

uint32_t QtImage::width() const
{
    return mImage->width();
}

uint32_t QtImage::height() const
{
    return mImage->height();
} 
