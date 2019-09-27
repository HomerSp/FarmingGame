#include <engine/graphics/image.h>
#include <engine/graphics/texture.h>

using namespace engine;
using namespace engine::graphics;

Texture::Texture(uint32_t /*w*/, uint32_t /*h*/, uint32_t layers)
    : mLayers(layers)
{
}

uint32_t Texture::layers() const
{
    return mLayers;
}

uint32_t Texture::width(uint32_t layer)
{
    return mDimensions[layer].width;
}

uint32_t Texture::height(uint32_t layer)
{
    return mDimensions[layer].height;
}

Types::Dimension<> Texture::dimension(uint32_t layer)
{
    return mDimensions[layer];
}

bool Texture::operator!() const
{
    return width() == 0;
}

void Texture::setDimensions(const Types::Dimension<>& dimen, uint32_t layer)
{
    mDimensions[layer] = dimen;
}
