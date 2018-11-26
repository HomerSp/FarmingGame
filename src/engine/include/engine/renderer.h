#pragma once

#include <cmath>
#include <memory>
#include <vector>

#include <engine/types.h>

namespace engine {

class Image;

class Renderer {
public:
    Renderer() = default;

    virtual int32_t width() = 0;
    virtual int32_t height() = 0;

    virtual void fillRect(const Types::Rect<>& dst, const Types::Color& color) = 0;

    virtual void drawImage(const Image& img, const Types::Rect<>& dst, const Types::Rect<>& src) = 0;
    virtual void drawText(const Types::Rect<>& dst, const std::string& text, const Types::Color& color, int32_t size = -1, Types::TextAlign align = Types::TextAlign(), std::string type = "") = 0;
    virtual void drawOverlay(const Types::Point<>& dst, const Types::Overlay& overlay) = 0;

    virtual void translate(float_t x, float_t y) = 0;

    virtual void save() = 0;
    virtual void restore() = 0;

    virtual std::shared_ptr<Image> loadImage(const std::string& path) = 0;

private:
};
}
