#pragma once

#include <memory>
#include <vector>

#include <engine/types.h>

namespace engine {
class Image {
public:
    Image();
    Image(const Types::Dimension<uint32_t> &d);
    Image(const std::string& path);
    
    const unsigned char* data() const;
    uint32_t dataSize() const;
    uint32_t width() const;
    uint32_t height() const;

    void setDimensions(const Types::Dimension<uint32_t> &d);

    bool operator!() const
    {
        return mDimen.width == 0;
    }

private:
	Types::Dimension<uint32_t> mDimen;
    std::vector<unsigned char> mData;
};
}
