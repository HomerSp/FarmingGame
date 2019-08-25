#include <engine/graphics/image.h>

using namespace engine::graphics;

bool Image::operator!() const
{
    return width() == 0;
}
