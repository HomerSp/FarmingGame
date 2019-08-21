#include <engine/graphics/color.h> 

using namespace engine::graphics;

constexpr uint32_t Color::Size;

Color::Color(const Color& o, float_t a)
    : Color(o.mData[0], o.mData[1], o.mData[2], a)
{
}

Color::Color(float_t r, float_t g, float_t b, float_t a)
    : mData({r, g, b, a})
{
}

const float_t* Color::constData() const
{
    return &mData[0];
}

float_t& Color::r()
{
    return mData[0];
}

float_t& Color::g()
{
    return mData[1];
}

float_t& Color::b()
{
    return mData[2];
}

float_t& Color::a()
{
    return mData[3];
}

const float_t& Color::r() const
{
    return mData[0];
}

const float_t& Color::g() const
{
    return mData[1];
}

const float_t& Color::b() const
{
    return mData[2];
}

const float_t& Color::a() const
{
    return mData[3];
}

Color& Color::operator*=(float_t alpha)
{
    mData[0] *= alpha;
    mData[1] *= alpha;
    mData[2] *= alpha;
    mData[3] = 1.0f;
    return *this;
}

Color Color::fromInt(uint8_t r, uint8_t g, uint8_t b, uint8_t a)
{
    return {r / 255.0f, g / 255.0f, b / 255.0f, a / 255.0f};
}

Color Color::max(const Color& first, const Color& second)
{
    return {
        std::max(first.mData[0], second.mData[0]),
        std::max(first.mData[1], second.mData[1]),
        std::max(first.mData[2], second.mData[2]),
        std::max(first.mData[3], second.mData[3])
    };
}