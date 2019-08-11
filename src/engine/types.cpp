#include <cassert>

#include <engine/types.h>

using namespace engine;

Types::Cells::Cells(uint32_t cols, uint32_t rows)
    : cols(cols)
    , rows(rows)
{
}

Types::Color::Color(const Color& o, float_t a)
    : Color(o.r, o.g, o.b, a)
{
}

Types::Color::Color(float_t r, float_t g, float_t b, float_t a)
    : r(r)
    , g(g)
    , b(b)
    , a(a)
{
}

Types::Color Types::Color::fromInt(uint8_t r, uint8_t g, uint8_t b, uint8_t a)
{
    return {r / 255.0f, g / 255.0f, b / 255.0f, a / 255.0f};
}

Types::ColorGradient::ColorGradient(const ColorGradient& other, float_t alpha)
    : inner({other.inner, alpha})
    , outer({other.outer, alpha})
{
}

Types::ColorGradient::ColorGradient(const Color& color)
    : ColorGradient(color, color)
{
}

Types::ColorGradient::ColorGradient(const Color& cInner, const Color& cOuter)
    : inner(cInner)
    , outer(cOuter)
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

Types::Overlay::Overlay(uint32_t w, uint32_t h, Types::Color bg)
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

Types::AtomicF::AtomicF(float f)
    : std::atomic<float>(f)
{
}

Types::AtomicF& Types::AtomicF::operator=(float d)
{
    store(d);
    return *this;
}

Types::AtomicF& Types::AtomicF::operator+=(float d)
{
    float old = load(std::memory_order_consume);
    float desired = old + d;
    while (!compare_exchange_weak(old, desired, std::memory_order_release, std::memory_order_consume))
    {
        desired = old + d;
    }

    return *this;
}