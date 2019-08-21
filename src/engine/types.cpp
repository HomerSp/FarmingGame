#include <cassert>

#include <engine/types.h>

using namespace engine;

Types::Cells::Cells(uint32_t cols, uint32_t rows)
    : cols(cols)
    , rows(rows)
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

Types::FilledEllipse::FilledEllipse(int32_t x, int32_t y, int32_t radius, const graphics::ColorGradient& gradient)
    : Ellipse(x, y, radius)
    , gradient(gradient)
{
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

double_t Types::PI()
{
    static double_t sPI = std::acos(-1);
    return sPI;
}