#include <engine/types.h>

using namespace engine;

Types::Cells::Cells(uint32_t cols, uint32_t rows)
    : cols(cols)
    , rows(rows)
{
}

Types::Color::Color(uint8_t r, uint8_t g, uint8_t b, uint8_t a)
    : r(r)
    , g(g)
    , b(b)
    , a(a)
{
}

Types::Pair::Pair(int first, int second)
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

Types::FilledEllipse::FilledEllipse(int32_t x, int32_t y, int32_t radius, Types::Color c)
    : Ellipse(x, y, radius)
    , color(std::move(c))
{
}

Types::Overlay::Overlay(uint32_t w, uint32_t h, Types::Color bg)
    : width(w)
    , height(h)
    , background(std::move(bg))
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