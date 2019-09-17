#include <engine/graphics/image.h>
#include <engine/graphics/texture.h>

using namespace engine::graphics;

Texture::Texture(uint32_t /*w*/, uint32_t /*h*/, uint32_t layers)
    : mLayers(layers)
{
}

uint32_t Texture::layers() const
{
    return mLayers;
}

bool Texture::operator!() const
{
    return width() == 0;
}
