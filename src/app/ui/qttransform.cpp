#include <ui/qttransform.h>

void QtTransform::reset()
{
    mTransform.reset();
}

void QtTransform::rotate(float_t angle)
{
    mTransform.rotate(angle);
}

void QtTransform::rotateOrigin(float_t angle, float_t x, float_t y)
{
    mTransform.translate(x, y);
    mTransform.rotate(angle);
    mTransform.translate(-x, -y);
}

void QtTransform::scale(float_t x, float_t y)
{
    mTransform.scale(x, y);
}

void QtTransform::translate(float_t x, float_t y)
{
    mTransform.translate(x, y);
}

engine::graphics::Transform& QtTransform::operator*=(const Transform& o)
{
    const auto& native = dynamic_cast<const QtTransform&>(o);
    mTransform *= native.mTransform;
    return *this;
}

QtTransform::operator QMatrix4x4() const
{
    return {mTransform};
}