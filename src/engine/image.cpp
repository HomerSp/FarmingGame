#include <map>

#include <lodepng.h>

#include <engine/image.h>
#include <engine/logger.h>

using namespace engine;

Image::Image()
{
}

bool Image::operator!() const
{
    return width() == 0;
}
