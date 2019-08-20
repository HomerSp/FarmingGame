#pragma once

#include <cmath>

#include <QMatrix4x4>

#include <engine/graphics/matrix.h>

class QtMatrix : public engine::graphics::Matrix
{
public:
    QtMatrix() = default;
    
    virtual const float_t* constData() const override;

    virtual uint32_t size() override;

    virtual void reset() override;

    virtual void ortho(float_t left, float_t right, float_t top, float_t bottom, float_t near, float_t far) override;

    virtual void scale(float_t x, float_t y) override;
    virtual void scale(float_t x, float_t y, float_t z) override;

    virtual void translate(float_t x, float_t y) override;
    virtual void translate(float_t x, float_t y, float_t z) override;

    operator QMatrix4x4() const;

private:
    QMatrix4x4 mMatrix;
};
