#include <engine/types.h>

using namespace engine;

Types::Cells::Cells(int cols, int rows)
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