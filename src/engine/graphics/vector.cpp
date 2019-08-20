#include <engine/graphics/vector.h> 

using namespace engine::graphics;

constexpr uint32_t Vector2D::Size;
constexpr uint32_t Vector4D::Size;

const float_t* Vector2D::constData() const
{
    return &mData[0];
}

const float_t& Vector2D::operator[](uint32_t index) const
{
    return mData.at(index);
}

float_t& Vector2D::operator[](uint32_t index)
{
    return mData.at(index);
}

const float_t* Vector4D::constData() const
{
    return &mData[0];
}

const float_t& Vector4D::operator[](uint32_t index) const
{
    return mData.at(index);
}

float_t& Vector4D::operator[](uint32_t index)
{
    return mData.at(index);
}
