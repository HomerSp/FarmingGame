#include <engine/camera.h>
#include <engine/clock.h>
#include <engine/logger.h>
#include <engine/renderer.h>
#include <engine/screeneffects.h>

using namespace engine;

ScreenEffects::ScreenEffects()
    : mRadiusMod(2.0f)
{
}

void ScreenEffects::draw(Renderer& renderer, Clock& clock, Camera& camera, const std::vector<std::shared_ptr<LightSource>> &sources)
{
    uint32_t h = clock.hour();
    if (h >= clock.sunrise() && h < clock.sunset()) {
        return;
    }

    Types::Color color(0, 0, 0, 0);

    // Night
    if (h < clock.dawn() || h >= clock.dusk()) {
        color.a = 175;
    // Sunrise
    } else if (h >= clock.dawn() && h < clock.sunrise()) {
        float diff = ((static_cast<uint64_t>(std::floor(clock.current())) % (24 * 60)) - (clock.dawn() * 60)) / static_cast<float>((clock.sunrise() - clock.dawn()) * 60);
        float alpha = 175 - 175 * diff;
        color.r = 150 * diff;
        color.a = std::floor(alpha);
    // Sunset
    } else if (h >= clock.sunset() && h < clock.dusk()) {
        float diff = ((static_cast<uint64_t>(std::floor(clock.current())) % (24 * 60)) - (clock.sunset() * 60)) / static_cast<float>((clock.dusk() - clock.sunset()) * 60);
        float alpha = 175 * diff;
        color.r = 150 - 150 * diff;
        color.a = std::floor(alpha);
    }

    int32_t radiusMod = std::abs(static_cast<int32_t>((mRadiusMod - 1.0f) * 32.0f));

    Types::Overlay overlay(renderer.width(), renderer.height(), color);
    for (auto& source: sources) {
        Types::Color l = color;
        l.r = l.r * (1.0f - source->strength());
        l.a = l.a * (1.0f - source->strength());
        overlay.addEllipse(Types::FilledEllipse(source->position().x - camera.x(), source->position().y - camera.y(), source->radius() + radiusMod, l));
    }

    renderer.drawOverlay({0, 0}, overlay);
}

bool ScreenEffects::processAsync(uint64_t frameDiff)
{
    float m = mRadiusMod - (frameDiff / 1000.0f);
    if (m < 0.0f) {
        m = 2.0f;
    }

    bool changed = std::floor(m) != std::floor(mRadiusMod);
    mRadiusMod = m;
    return changed;
}