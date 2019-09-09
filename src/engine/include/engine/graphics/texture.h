#pragma once

#include <cstdint>

namespace engine {
namespace graphics {

class Image;

class Texture {
public:
    Texture(uint32_t w, uint32_t h, uint32_t layers = 0);
    virtual ~Texture() = default;

    virtual void bind(uint32_t id = 0) = 0;
    virtual void release() = 0;

    virtual void setData(const Image& image, uint32_t layer = 0) = 0;
    
    virtual uint32_t width() const = 0;
    virtual uint32_t height() const = 0;

    bool operator!() const;
};
}
}
