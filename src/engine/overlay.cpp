#include <engine/graphics/bufferwriter.h>
#include <engine/graphics/colorgradient.h>
#include <engine/graphics/matrix.h>
#include <engine/overlay.h>

using namespace engine;

graphics::ColorGradient Overlay::LightSource::lightColor()   
{
    return {{1.0f, 1.0f, 1.0f}};
}

Overlay::Overlay(graphics::Renderer& renderer, uint32_t lightsCount)
    : mRadiusMod(2.0f)
    , mBackground({0, 0, 0, 0})
{
    mBuffer = renderer.createBuffer((graphics::ColorGradient::Size() + graphics::Matrix::Size()) * lightsCount);
}

const graphics::Color& Overlay::background() const
{
    return mBackground;
}

graphics::Buffer& Overlay::buffer()
{
    return *mBuffer;
}

void Overlay::draw(graphics::Renderer& renderer, Camera& camera, const std::vector<std::shared_ptr<LightSource>> &sources)
{
    uint32_t lightsCount = 0;

    graphics::BufferWriter writer(*mBuffer);
    for (auto& source: sources) {
        auto radius = source->lightRadius();
        if (radius == 0.0f) {
            continue;
        }

        auto l = source->lightColor();
        writer += engine::graphics::Color::max(l.inner(), mBackground);
        writer += engine::graphics::Color::max(l.outer(), mBackground);

        auto pos = source->lightPosition();
        auto matrix = renderer.createMatrix();
        matrix->translate(pos.x - (radius / 2.0f) - camera.x(), pos.y - (radius / 2.0f) - camera.y());
        matrix->scale(radius, radius);
        writer += *matrix;

        lightsCount++;
    }

    renderer.drawOverlay({0, 0}, *this, lightsCount, mRadiusMod);
}

bool Overlay::processAsync(uint64_t frameDiff)
{
    float_t m = mRadiusMod - (frameDiff / 1000.0f);
    if (m < 0.0f) {
        m = 2.0f;
    }

    bool changed = std::floor(m * 8.0f) != std::floor(mRadiusMod * 8.0f);
    mRadiusMod = m;
    return changed;
}

void Overlay::setBackground(const graphics::Color& bg)
{
    mBackground = bg;
}