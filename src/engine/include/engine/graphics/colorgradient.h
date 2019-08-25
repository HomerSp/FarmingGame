#pragma once

#include <cstdint>

#include <engine/graphics/color.h>

namespace engine {
namespace graphics {
class ColorGradient {
public:
    static constexpr uint32_t Size = Color::Size * 2;

    ColorGradient(const ColorGradient& other, float_t alpha = 1.0f);
    ColorGradient(const Color& color);
    ColorGradient(const Color& cInner, const Color& cOuter);

    const Color& inner() const;
    const Color& outer() const;

    Color& inner();
    Color& outer();

    ColorGradient& operator*=(float_t alpha);

private:
    Color mInner, mOuter;
};
}
}
