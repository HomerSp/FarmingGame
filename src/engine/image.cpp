#include <map>

#include <engine/image.h>
#include <engine/logger.h>

using namespace engine;

bool Image::operator!() const
{
    return width() == 0;
}
