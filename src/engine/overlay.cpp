#include <engine/graphics/colorgradient.h>
#include <engine/graphics/vector.h>
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
    mLightsBuffer = renderer.createBuffer((graphics::ColorGradient::Size() + graphics::Vector4D::Size()) * lightsCount);
}

const graphics::Color& Overlay::background() const
{
    return mBackground;
}

graphics::Buffer& Overlay::lightsBuffer()
{
    return *mLightsBuffer;
}

void Overlay::draw(graphics::Renderer& renderer, const Types::Point<> dst, const std::vector<std::shared_ptr<LightSource>> &sources)
{
    uint32_t lightsCount = 0;

    auto writer = mLightsBuffer->writer();
    for (auto& source: sources) {
        auto radius = source->lightRadius();
        if (radius == 0.0f) {
            continue;
        }

        auto l = source->lightColor();
        writer.append(engine::graphics::Color::max(l.inner(), mBackground));
        writer.append(engine::graphics::Color::max(l.outer(), mBackground));

        auto pos = source->lightPosition();
        writer.append(engine::graphics::Vector4D(pos.x - (radius / 2.0f), pos.y - (radius / 2.0f), radius, radius));

        lightsCount++;
    }

    writer.release();

    renderer.drawOverlay(dst, *this, lightsCount, mRadiusMod);
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