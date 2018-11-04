#include <png++/png.hpp>

#include "include/utils.h"

int main(int argc, char** argv)
{
	if(argc != 3) {
		return -1;
	}

	png::image<png::gray_pixel, png::pixel_buffer<png::gray_pixel> > image(argv[1]);
	if(image.get_width() == 0) {
		return -1;
	}

	Utils::createParentDir(argv[2]);

	png::image<png::gray_pixel_1> out(image.get_width(), image.get_height());
	for (size_t j = 0; j < image.get_height(); ++j)
    {
        for (size_t i = 0; i < image.get_width(); ++i)
        {
            int val = (image[j][i] != 255) ? 0 : 1;
            out.set_pixel(i, j, png::gray_pixel_1(val));
        }
    }

	out.write(argv[2]);

	return 0;
} 
