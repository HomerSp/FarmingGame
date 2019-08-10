#include <engine/types.h>

using namespace engine;

Types::Cells::Cells(uint32_t cols, uint32_t rows)
    : cols(cols)
    , rows(rows)
{
}

Types::Color::Color(const Color& o, uint8_t a)
    : Color(o.r, o.g, o.b, a)
{
}

Types::Color::Color(uint8_t r, uint8_t g, uint8_t b, uint8_t a)
    : r(r)
    , g(g)
    , b(b)
    , a(a)
{
}

Types::ColorF::ColorF(const Color& o)
    : ColorF(o.r / 255.0f, o.g / 255.0f, o.b / 255.0f, o.a / 255.0f)
{

}

Types::ColorF::ColorF(const ColorF& o, float_t a)
    : ColorF(o.r, o.g, o.b, a)
{
}

Types::ColorF::ColorF(float_t r, float_t g, float_t b, float_t a)
    : r(r)
    , g(g)
    , b(b)
    , a(a)
{
}

Types::ColorGradient::ColorGradient(const ColorGradient& other, float_t alpha)
    : inner({other.inner, alpha})
    , outer({other.outer, alpha})
{
}

Types::ColorGradient::ColorGradient(const ColorF& color)
    : ColorGradient(color, color)
{
}

Types::ColorGradient::ColorGradient(const ColorF& cInner, const ColorF& cOuter)
    : inner(cInner)
    , outer(cOuter)
{
}

Types::ColorGradient::ColorGradient(const Color& color)
    : ColorGradient(color, color)
{
}

Types::ColorGradient::ColorGradient(const Color& cInner, const Color& cOuter)
    : inner(Types::ColorF(cInner))
    , outer(Types::ColorF(cOuter))
{
}

Types::Pair::Pair(int32_t first, int32_t second)
    : first(first)
    , second(second)
{
}

Types::Ellipse::Ellipse(int32_t x, int32_t y, int32_t radius)
    : x(x)
    , y(y)
    , radius(radius)
{
}

Types::FilledEllipse::FilledEllipse(int32_t x, int32_t y, int32_t radius, const Types::ColorGradient& gradient)
    : Ellipse(x, y, radius)
    , gradient(gradient)
{
}

Types::Overlay::Overlay(uint32_t w, uint32_t h, Types::ColorF bg)
    : width(w)
    , height(h)
    , background(bg)
{
}

void Types::Overlay::addEllipse(const Types::FilledEllipse &ellipse)
{
    ellipses.push_back(ellipse);
}

Types::TextAlign::TextAlign(std::initializer_list<Types::TextAlign::Type> types)
{
    for (auto t: types) {
        bits[t] = true;
    }

    if (!bits[Left] && !bits[Right] && !bits[CentreH]) {
        bits[Left] = true;
    }

    if (!bits[Top] && !bits[Bottom] && !bits[CentreV]) {
        bits[Top] = true;
    }
}

bool Types::TextAlign::is(Type t) const
{
    return bits[t];
}