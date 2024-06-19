#pragma once

#include <cstdint>
#include <string>
#include <vector>

#include <engine/types.h>

namespace engine {
namespace graphics {
class Image {
public:
    Image();
    Image(const Image& other);
    Image(const std::string& path);
    Image(const std::vector<uint8_t>& data);
    Image(const uint8_t* data, uint32_t width, uint32_t height);
    ~Image() = default;

    Image copy(const Types::Rect<uint32_t> &rc) const;

    const uint8_t *data() const;
    
    uint32_t width() const;
    uint32_t height() const;

    bool operator!() const;

private:
    void initData(const std::vector<uint8_t>& data);

    uint32_t mWidth, mHeight;
    std::vector<uint8_t> mData;
};
}
}
