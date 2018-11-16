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
    void fillEllipse(const engine::Types::Ellipse& dst, const engine::Types::Color& fromColor, const engine::Types::Color& toColor);

    void drawImage(const engine::Image& img, const engine::Types::Rect<>& src, const engine::Types::Rect<>& dst);
    void drawText(const engine::Types::Point<>& dst, const std::string& text, const engine::Types::Color& color, int size, engine::Types::TextAlign align);
    void drawOverlay(const engine::Types::Point<>& dst, const engine::Types::Overlay& overlay);

    void translate(float x, float y);

    void save();
    void restore();

    void setClipEllipses(const std::vector<engine::Types::Ellipse>& dst);
    void setClipEllipsesScreen(const std::vector<engine::Types::Ellipse>& dst, const engine::Types::Rect<>& rc);

    void setPainter(QPainter* painter);

    std::shared_ptr<engine::Image> loadImage(const std::string& path);

private:
    QPainter* mPainter;
};
