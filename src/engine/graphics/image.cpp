#include <lodepng.h>

#include <engine/graphics/image.h>
#include <engine/logger.h>

using namespace engine::graphics;

Image::Image()
    : mWidth(0)
    , mHeight(0)
{
}

Image::Image(const Image& other)
    : mWidth(other.mWidth)
    , mHeight(other.mHeight)
{
    std::copy(other.mData.begin(), other.mData.end(), std::back_inserter(mData));
}

Image::Image(const std::string& path)
    : Image()
{
    std::vector<uint8_t> fileData;
    lodepng::load_file(fileData, path);
    initData(fileData);
}

Image::Image(const std::vector<uint8_t>& data)
    : Image()
{
    initData(data);
}

Image::Image(const uint8_t* data, uint32_t width, uint32_t height)
    : mWidth(width)
    , mHeight(height)
{
    std::copy(data, data + (width * height * 4), std::back_inserter(mData));
}

Image Image::copy(const Types::Rect<uint32_t> &rc) const
{
    if (rc.top() >= rc.bottom() || rc.left() >= rc.right()) {
        return Image();
    }

    Image ret;
    ret.mWidth = rc.width;
    ret.mHeight = rc.height;
    ret.mData.resize(rc.width * rc.height * 4, 0);

    uint32_t colCount = ((mWidth < rc.width) ? mWidth : rc.width) * 4;
    for (uint32_t row = rc.top(); row < static_cast<uint32_t>(rc.bottom()); row++) {
        uint32_t toStart = row * rc.width * 4;
        uint32_t fromStart = (rc.x + row * mWidth) * 4;
        std::copy(mData.begin() + fromStart, mData.begin() + fromStart + colCount, ret.mData.begin() + toStart);
    }

    return ret;
}

const uint8_t *Image::data() const
{
    return mData.data();
}

uint32_t Image::width() const
{
    return mWidth;
}

uint32_t Image::height() const
{
    return mHeight;
}

bool Image::operator!() const
{
    return mWidth == 0;
}

void Image::initData(const std::vector<uint8_t>& data)
{
    if (lodepng::decode(mData, mWidth, mHeight, data) != 0) {
        mWidth = 0;
    }
}
