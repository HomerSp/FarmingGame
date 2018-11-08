#include <lodepng.h>

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
    std::vector<unsigned char> data;
    std::vector<unsigned char> buffer;

    if (lodepng::load_file(buffer, path) == 0) {
        return;
    }

    if (lodepng::decode(data, mWidth, mHeight, buffer) == 0) {
        return;
    }

    mData.reserve(data.size());
    for (uint32_t i = 0; i < data.size(); i += 4) {
        mData.push_back(data.at(i + 2));    // Blue
        mData.push_back(data.at(i + 1));    // Green
        mData.push_back(data.at(i + 0));    // Red
        mData.push_back(data.at(i + 3));    // Alpha
    }
}
