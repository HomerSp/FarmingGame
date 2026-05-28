#pragma once

#include <cstdint>
#include <map>

namespace engine {
namespace graphics {

class Image;

class Texture {
public:
    Texture(uint32_t w, uint32_t h, uint32_t layers = 0);
    virtual ~Texture() = default;

    uint32_t layers() const;
    uint32_t width(uint32_t layer);
    uint32_t height(uint32_t layer);
    Types::Dimension<> dimension(uint32_t layer);

    bool operator!() const;



    virtual void setData(const Image& image, uint32_t layer) = 0;
    
    virtual uint32_t width() const = 0;
    virtual uint32_t height() const = 0;

protected:
    void setDimensions(const Types::Dimension<>& dimen, uint32_t layer);

private:
    std::map<uint32_t, Types::Dimension<>> mDimensions;
    uint32_t mLayers;
};
}
}
