#pragma once

#include <array>
#include <cmath>
#include <cstdint>

namespace engine {
namespace graphics {
class Matrix {
private:
    enum {
        Identity        = 0x0000,
        Translation     = 0x0001,
        Scale           = 0x0002,
        Rotation        = 0x0004,
    };
public:
    static constexpr inline uint32_t Size() { return sizeof(float_t) * 4 * 4; };

    Matrix();
    ~Matrix() = default;

    const float_t* constData() const;

    void reset();

    void ortho(float_t left, float_t right, float_t top, float_t bottom, float_t near, float_t far);

    void rotate(float_t angle);

    void scale(float_t x, float_t y);

    void translate(float_t x, float_t y);

    Matrix& operator=(const Matrix& o);
    Matrix& operator*=(const Matrix& o);

private:
    Matrix(uint16_t flags);

    void multiplyRow(const Matrix& o, uint8_t r);

    uint16_t mFlags;
    std::array<std::array<float_t, 4>, 4> mValues;
};
}
}
