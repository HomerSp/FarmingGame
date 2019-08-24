#include <QTransform>

#include <ui/qtmatrix.h>

const float_t* QtMatrix::constData() const
{
    return mMatrix.constData();
}

void QtMatrix::reset()
{
    mMatrix.setToIdentity();
}

void QtMatrix::ortho(float_t left, float_t right, float_t top, float_t bottom, float_t near, float_t far)
{
    mMatrix.ortho(left, right, top, bottom, near, far);
}

void QtMatrix::rotate(float_t angle, float_t z)
{
    mMatrix.rotate(angle, 0.0f, 0.0f, z);
}

void QtMatrix::scale(float_t x, float_t y)
{
    mMatrix.scale(x, y);
}

void QtMatrix::scale(float_t x, float_t y, float_t z)
{
    mMatrix.scale(x, y, z);
}

void QtMatrix::translate(float_t x, float_t y)
{
    mMatrix.translate(x, y);
}

void QtMatrix::translate(float_t x, float_t y, float_t z)
{
    mMatrix.translate(x, y, z);
}

engine::graphics::Matrix& QtMatrix::operator*=(const Matrix& o)
{
    const auto& native = dynamic_cast<const QtMatrix&>(o);
    mMatrix *= native.mMatrix;
    return *this;
}

QtMatrix::operator QMatrix4x4() const
{
    return mMatrix;
}