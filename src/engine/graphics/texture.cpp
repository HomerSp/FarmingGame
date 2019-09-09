#include <engine/graphics/image.h>
#include <engine/graphics/texture.h>

using namespace engine::graphics;

Texture::Texture(uint32_t /*w*/, uint32_t /*h*/, uint32_t /*layers*/)
{
}

bool Texture::operator!() const
{
    return width() == 0;
}
