#include <engine/graphics/matrix.h>
#include <engine/types.h>

using namespace engine;
using namespace engine::graphics;

Matrix::Matrix()
    : mFlags(Identity)
    , mValues({})
{
    reset();
}

const float_t* Matrix::constData() const
{
    return &mValues[0][0];
}

void Matrix::reset()
{
    mValues.fill({0.0f, 0.0f, 0.0f, 0.0f});
    mValues[0][0] = 1.0f;
    mValues[1][1] = 1.0f;
    mValues[2][2] = 1.0f;
    mValues[3][3] = 1.0f;
    mFlags = Identity;
}

void Matrix::ortho(float_t left, float_t right, float_t top, float_t bottom, float_t near, float_t far)
{
    float width = right - left;
    float invheight = top - bottom;
    float clip = far - near;
    Matrix m(Translation | Scale);
    m.mValues[0][0] = 2.0f / width;
    m.mValues[1][1] = 2.0f / invheight;
    m.mValues[2][2] = -2.0f / clip;
    m.mValues[3][0] = -(left + right) / width;
    m.mValues[3][1] = -(top + bottom) / invheight;
    m.mValues[3][2] = -(near + far) / clip;
    m.mValues[3][3] = 1.0f;
   *this *= m;
}

void Matrix::rotate(float_t angle)
{
    float_t radians = angle * Types::PI() / 180.0f;
    float_t c = std::cos(radians);
    float_t s = std::sin(radians);
    for (int r = 0; r < 4; r++) {
        float tmp = mValues[0].at(r);
        mValues[0].at(r) = tmp * c + mValues[1].at(r) * s;
        mValues[1].at(r) = mValues[1].at(r) * c - tmp * s;
    }

    mFlags |= Rotation;
}

void Matrix::scale(float_t x, float_t y)
{
    if (mFlags < Scale) {
        mValues[0][0] = x;
        mValues[1][1] = y;
    } else if (mFlags < Rotation) {
        mValues[0][0] *= x;
        mValues[1][1] *= y;
    } else {
        mValues[0][0] *= x;
        mValues[0][1] *= x;
        mValues[1][0] *= y;
        mValues[1][1] *= y;
    }

    mFlags |= Scale;
}

void Matrix::translate(float_t x, float_t y)
{
    switch (mFlags) {
    case Identity:
        mValues[3][0] = x;
        mValues[3][1] = y;
        break;
    case Translation:
        mValues[3][0] += x;
        mValues[3][1] += y;
        break;
    case Scale:
        mValues[3][0] = mValues[0][0] * x;
        mValues[3][1] = mValues[1][1] * y;
        break;
    case Translation | Scale:
        mValues[3][0] += mValues[0][0] * x;
        mValues[3][1] += mValues[1][1] * y;
        break;
    default:
        mValues[3][0] += mValues[0][0] * x + mValues[1][0] * y;
        mValues[3][1] += mValues[0][1] * x + mValues[1][1] * y;
        break;
    }

    mFlags |= Translation;
}

Matrix& Matrix::operator=(const Matrix& o)
{
    mFlags = o.mFlags;
    std::copy(&o.mValues[0][0], &o.mValues[3][3], &mValues[0][0]);
    return *this;
}

Matrix& Matrix::operator*=(const Matrix& o)
{
    if (mFlags < Rotation) {
        mValues[3][0] += mValues[0][0] * o.mValues[3][0];
        mValues[3][1] += mValues[1][1] * o.mValues[3][1];
        mValues[3][2] += mValues[2][2] * o.mValues[3][2];
        mValues[0][0] *= o.mValues[0][0];
        mValues[1][1] *= o.mValues[1][1];
        mValues[2][2] *= o.mValues[2][2];
    } else {
        for (int r = 0; r < 4; r++) {
            multiplyRow(o, r);
        }
    }

    mFlags |= o.mFlags;

    return *this;
}

Matrix::Matrix(uint16_t flags)
    : mFlags(flags)
    , mValues({})
{
    mValues.fill({0.0f, 0.0f, 0.0f, 0.0f});
}

void Matrix::multiplyRow(const Matrix& o, uint8_t r)
{
    std::array<float_t, 4> ms = {0.0f, 0.0f, 0.0f, 0.0f};

    for (int m = 0; m < 4; m++) {
        for (int i = 0; i < 4; i++) {
            ms.at(m) += mValues.at(i).at(r) * o.mValues.at(m).at(i);
        }
    }

    for (int m = 0; m < 4; m++) {
        mValues.at(m).at(r) = ms.at(m);
    }
}