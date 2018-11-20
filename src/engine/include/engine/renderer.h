#pragma once

#include <memory>
#include <vector>

#include <engine/types.h>

namespace engine {

class Image;

class Renderer {
public:
    Renderer() = default;

    virtual int width() = 0;
    virtual int height() = 0;

    virtual void fillRect(const Types::Rect<>& dst, const Types::Color& color) = 0;

    virtual void drawImage(const Image& img, const Types::Rect<>& dst, const Types::Rect<>& src) = 0;
    virtual void drawText(const Types::Point<>& dst, const std::string& text, const Types::Color& color, int size = -1, Types::TextAlign align = Types::TextAlign()) = 0;
    virtual void drawOverlay(const Types::Point<>& dst, const Types::Overlay& overlay) = 0;

    virtual void translate(float x, float y) = 0;

    virtual void save() = 0;
    virtual void restore() = 0;

    virtual std::shared_ptr<Image> loadImage(const std::string& path) = 0;

private:
};
}
