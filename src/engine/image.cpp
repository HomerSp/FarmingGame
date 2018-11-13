#include <map>

#include <lodepng.h>

#include <engine/image.h>
#include <engine/logger.h>

using namespace engine;

Image::Image()
    : Image(Types::Dimension<uint32_t>(0, 0))
{
}

Image::Image(const Types::Dimension<uint32_t>& d)
    : mDimen(d.width, d.height)
{
    mData.resize(mDimen.width * mDimen.height * 4);
}

Image::Image(const std::string& path)
    : Image()
{
    std::vector<unsigned char> data;
    std::vector<unsigned char> buffer;

    if (lodepng::load_file(buffer, path) != 0) {
        return;
    }

    if (lodepng::decode(data, mDimen.width, mDimen.height, buffer) != 0) {
        return;
    }

    mData.resize(mDimen.width * mDimen.height * 4);
    for (uint32_t i = 0; i < mDimen.width * mDimen.height * 4; i += 4) {
        mData[i + 0] = data.at(i + 2);    // Blue
        mData[i + 1] = data.at(i + 1);    // Green
        mData[i + 2] = data.at(i + 0);    // Red
        mData[i + 3] = data.at(i + 3);    // Alpha
    }
}

const unsigned char* Image::data() const
{
    return mData.data();
}

uint32_t Image::dataSize() const
{
    return mDimen.width * mDimen.height * 4;
}

uint32_t Image::width() const
{
    return mDimen.width;
}

uint32_t Image::height() const
{
    return mDimen.height;
}

void Image::setDimensions(const Types::Dimension<uint32_t> &d)
{
    mDimen = d;
    mData.resize(mDimen.width * mDimen.height * 4);
}
