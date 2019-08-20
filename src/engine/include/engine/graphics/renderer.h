#pragma once

#include <cmath>
#include <memory>
#include <vector>

#include <engine/particles.h>
#include <engine/types.h>

namespace engine {

class Context;
class Engine;
class Particles;

namespace graphics {

class Image;

class Renderer {
public:
    Renderer() = default;

    virtual void setSize(uint32_t w, uint32_t h, double devicePixelRatio) = 0;

    virtual void paint(std::shared_ptr<engine::Engine>& engine) = 0;

    virtual int32_t width() = 0;
    virtual int32_t height() = 0;

    virtual void fillEllipse(const engine::Types::Rect<>& dst, const engine::graphics::Color& color) = 0;
    virtual void fillRect(const Types::Rect<>& dst, const graphics::Color& color) = 0;

    virtual void drawImage(const graphics::Image& img, Types::Rect<> dst = Types::Rect<>(), Types::Rect<> src = Types::Rect<>()) = 0;
    virtual void drawText(const Types::Rect<>& dst, const std::string& text, const graphics::Color& color, int32_t size = -1, Types::TextAlign align = Types::TextAlign(), std::string type = "") = 0;
    virtual void drawOverlay(const Types::Point<>& dst, const Types::Overlay& overlay, float mod) = 0;
    virtual void drawParticles(const engine::Types::Point<>& dst, const engine::Particles& particles) = 0;

    virtual void rotate(float_t deg) = 0;
    virtual void translate(int32_t x, int32_t y) = 0;

    virtual void save() = 0;
    virtual void restore() = 0;

    virtual std::unique_ptr<graphics::Image> loadImage(const std::string& path) const = 0;

    Context& context();

protected:
    friend class engine::Engine;

    virtual void initContext() = 0;
    void setContext(std::shared_ptr<Context>& ctx);

private:
    std::shared_ptr<Context> mContext;
};
}
}
