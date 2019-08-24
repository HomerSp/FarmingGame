#pragma once

#include <cmath>

#include <QMatrix4x4>
#include <QTransform>

#include <engine/graphics/transform.h>

class QtTransform : public engine::graphics::Transform
{
public:
    QtTransform() = default;
    
    virtual void reset() override;

    virtual void rotate(float_t angle) override;
    virtual void rotateOrigin(float_t angle, float_t x, float_t y) override;

    virtual void scale(float_t x, float_t y) override;

    virtual void translate(float_t x, float_t y) override;

    virtual Transform& operator*=(const Transform& o) override;

    operator QMatrix4x4() const;

private:
    QTransform mTransform;
};
