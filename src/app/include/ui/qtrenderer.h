#pragma once

#include <QImage>
#include <QPainter>

#include <memory>

#include <engine/image.h>
#include <engine/renderer.h>

class QtRenderer : public engine::Renderer {
public:
    class QtImage : public engine::Image {
    public:
        QtImage(const std::string& path);
        virtual ~QtImage() = default;

        const QImage& image() const;

        virtual uint32_t width() const;
        virtual uint32_t height() const;

    private:
        std::unique_ptr<QImage> mImage;
    };

public:
    QtRenderer();
    virtual ~QtRenderer() = default;

    void init();

    int32_t width();
    int32_t height();

    void fillEllipse(const engine::Types::Rect<>& dst, const engine::Types::Color& color);
    void fillRect(const engine::Types::Rect<>& dst, const engine::Types::Color& color);

    void drawImage(const engine::Image& img, engine::Types::Rect<> dst, engine::Types::Rect<> src);
    void drawText(const engine::Types::Rect<>& dst, const std::string& text, const engine::Types::Color& color, int32_t size, engine::Types::TextAlign align, std::string type);
    void drawOverlay(const engine::Types::Point<>& dst, const engine::Types::Overlay& overlay);

    void rotate(float_t deg);
    void translate(int32_t x, int32_t y);

    void save();
    void restore();

    void setPainter(QPainter* painter);

    std::unique_ptr<engine::Image> loadImage(const std::string& path) const;

private:
    QPainter* mPainter;
};
