#include <map>

#include <engine/graphics/image.h>
#include <engine/logger.h>

using namespace engine::graphics;

bool Image::operator!() const
{
    return width() == 0;
}
