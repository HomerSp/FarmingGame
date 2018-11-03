#include <png++/png.hpp>

#include "include/utils.h"

int main(int argc, char** argv)
{
	if(argc != 3) {
		return -1;
	}

	png::image<png::rgba_pixel> png(argv[1]);
	if(png.get_width() == 0) {
		return -1;
	}

	Utils::createParentDir(argv[2]);

	bool hasAlpha = false;
	for(uint32_t y = 0; y < png.get_height(); y++) {
		for(uint32_t x = 0; x < png.get_width(); x++) {
			if(png[y][x].alpha != 255) {
				hasAlpha = true;
				break;
			}
		}
	}

	if(hasAlpha) {
		png::image<png::rgba_pixel, png::pixel_buffer<png::rgba_pixel> > out(argv[1]);
		out.write(argv[2]);
	} else {
		png::image<png::rgb_pixel, png::solid_pixel_buffer<png::rgb_pixel> > out(argv[1]);
		out.write(argv[2]);
	}

	return 0;
} 
