#pragma once

#include <QImage>
#include <QPainter>

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
        std::shared_ptr<QImage> mImage;
    };

public:
    QtRenderer();
    virtual ~QtRenderer() = default;

    int width();
    int height();

    void fillRect(const engine::Types::Rect<>& dst, const engine::Types::Color& color);

    void drawImage(const engine::Image& img, const engine::Types::Rect<>& dst, const engine::Types::Rect<>& src);
    void drawText(const engine::Types::Point<>& dst, const std::string& text, const engine::Types::Color& color, int size, engine::Types::TextAlign align);
    void drawOverlay(const engine::Types::Point<>& dst, const engine::Types::Overlay& overlay);

    void translate(float x, float y);

    void save();
    void restore();

    void setPainter(QPainter* painter);

    std::shared_ptr<engine::Image> loadImage(const std::string& path);

private:
    QPainter* mPainter;
};
