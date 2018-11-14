#pragma once

#include <memory>
#include <vector>

#include <engine/types.h>

namespace engine {
class Image {
public:
    Image();
    
    virtual uint32_t width() const = 0;
    virtual uint32_t height() const = 0;

    bool operator!() const;
};
}
