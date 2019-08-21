#include <engine/graphics/colorgradient.h> 

using namespace engine::graphics;

ColorGradient::ColorGradient(const ColorGradient& other, float_t alpha)
    : mInner(Color(other.mInner, alpha))
    , mOuter(Color(other.mOuter, alpha))
{
}

ColorGradient::ColorGradient(const Color& color)
    : ColorGradient(color, color)
{
}

ColorGradient::ColorGradient(const Color& cInner, const Color& cOuter)
    : mInner(cInner)
    , mOuter(cOuter)
{
}

const Color& ColorGradient::inner() const
{
    return mInner;
}

const Color& ColorGradient::outer() const
{
    return mOuter;
}

Color& ColorGradient::inner()
{
    return mInner;
}

Color& ColorGradient::outer()
{
    return mOuter;
}

ColorGradient& ColorGradient::operator*=(float_t alpha)
{
    mInner *= alpha;
    mOuter *= alpha;
    return *this;
}