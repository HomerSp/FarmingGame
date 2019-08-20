#pragma once

#include <memory>
#include <vector>

#include <engine/types.h>

namespace engine {
namespace graphics {
class Image {
public:
    Image() = default;
    virtual ~Image() = default;
    
    virtual uint32_t width() const = 0;
    virtual uint32_t height() const = 0;

    bool operator!() const;
};
}
}
