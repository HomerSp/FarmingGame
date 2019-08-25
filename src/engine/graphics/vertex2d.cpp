#include <engine/graphics/vector2d.h>
#include <engine/graphics/vertex2d.h> 

using namespace engine::graphics;

constexpr uint32_t Vertex2D::Size;

const float_t* Vertex2D::constData() const
{
    return &mData[0][0];
}

Vertex2D& Vertex2D::tl(float_t x, float_t y)
{
    mData[0][0] = x;
    mData[0][1] = y;
    return *this;
}

Vertex2D& Vertex2D::bl(float_t x, float_t y)
{
    mData[1][0] = x;
    mData[1][1] = y;
    return *this;
}

Vertex2D& Vertex2D::tr(float_t x, float_t y)
{
    mData[2][0] = x;
    mData[2][1] = y;
    return *this;
}

Vertex2D& Vertex2D::br(float_t x, float_t y)
{
    mData[3][0] = x;
    mData[3][1] = y;
    return *this;
}
