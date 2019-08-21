#pragma once

#include <cmath>
#include <cstdint>

#include <array>

namespace engine {
namespace graphics {
class Color {
public:
    static constexpr uint32_t Size = sizeof(float_t) * 4;

    Color(const Color& o, float_t a);
    Color(float_t r, float_t g, float_t b, float_t a = 1.0f);

    const float_t* constData() const;

    float_t& r();
    float_t& g();
    float_t& b();
    float_t& a();

    const float_t& r() const;
    const float_t& g() const;
    const float_t& b() const;
    const float_t& a() const;

    Color& operator*=(float_t alpha);

    static Color fromInt(uint8_t r, uint8_t g, uint8_t b, uint8_t a = 255);
    static Color max(const Color& first, const Color& second);

private:
    std::array<float_t, 4> mData;
};
}
}
