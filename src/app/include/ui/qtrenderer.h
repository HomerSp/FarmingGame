#pragma once

#include <QPainter>

#include <engine/renderer.h>

class QtRenderer : public engine::Renderer {
public:
    QtRenderer();
    virtual ~QtRenderer() = default;

    int width();
    int height();

    void fillRect(const engine::Types::Rect<>& dst, const engine::Types::Color& color);

    void drawImage(const engine::Image& img, const engine::Types::Rect<>& src, const engine::Types::Rect<>& dst);
    void drawText(const engine::Types::Point<>& dst, const std::string& text, const engine::Types::Color& color, int size, engine::Types::TextAlign align);

    void translate(float x, float y);

    void setPainter(QPainter* painter)
    {
        mPainter = painter;
    }

private:
    QPainter* mPainter;
};
