#include <engine/image.h>

using namespace engine;

Image::Image()
	: mWidth(0)
	, mHeight(0)
{

}

Image::Image(const std::string& path)
	: Image()
{
	png::image<png::rgba_pixel> png(path);
	mWidth = png.get_width();
	mHeight = png.get_height();

	mData = std::shared_ptr<uint8_t>(new uint8_t[mHeight * mWidth * 4]);

	uint8_t* data = mData.get();
	for(int y = 0; y < mHeight; y++) {
		auto row = png.get_row(y);
		for(int x = 0; x < mWidth; x++) {
			auto pixel = row[x];
			*(data++) = pixel.blue;
			*(data++) = pixel.green;
			*(data++) = pixel.red;
			*(data++) = pixel.alpha;
		}
	}
} 

Image::~Image() {

}