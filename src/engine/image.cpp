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

    mData.reserve(mWidth * mHeight * 4);
    for (uint32_t y = 0; y < mHeight; y++) {
        auto row = png.get_row(y);
        for (uint32_t x = 0; x < mWidth; x++) {
            auto pixel = row[x];
            mData.push_back(pixel.blue);
            mData.push_back(pixel.green);
            mData.push_back(pixel.red);
            mData.push_back(pixel.alpha);
        }
    }
}
