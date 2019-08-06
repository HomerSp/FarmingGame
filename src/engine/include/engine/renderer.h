#pragma once

#include <cmath>
#include <memory>
#include <vector>

#include <engine/types.h>

namespace engine {

class Context;
class Engine;
class Image;

class Renderer {
public:
    Renderer() = default;

    virtual void init() = 0;
    virtual void setSize(uint32_t w, uint32_t h) = 0;

    virtual int32_t width() = 0;
    virtual int32_t height() = 0;

    virtual void fillEllipse(const engine::Types::Rect<>& dst, const engine::Types::Color& color) = 0;
    virtual void fillRect(const Types::Rect<>& dst, const Types::Color& color) = 0;

    virtual void drawImage(const Image& img, Types::Rect<> dst = Types::Rect<>(), Types::Rect<> src = Types::Rect<>()) = 0;
    virtual void drawText(const Types::Rect<>& dst, const std::string& text, const Types::Color& color, int32_t size = -1, Types::TextAlign align = Types::TextAlign(), std::string type = "") = 0;
    virtual void drawOverlay(const Types::Point<>& dst, const Types::Overlay& overlay, float mod) = 0;

    virtual void rotate(float_t deg) = 0;
    virtual void translate(int32_t x, int32_t y) = 0;

    virtual void save() = 0;
    virtual void restore() = 0;

    virtual std::unique_ptr<Image> loadImage(const std::string& path) const = 0;

    Context& context();

protected:
    friend class Engine;

    void setContext(std::shared_ptr<Context>& ctx);

private:
    std::shared_ptr<Context> mContext;
};
}
