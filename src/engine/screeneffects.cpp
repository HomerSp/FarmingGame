#include <engine/camera.h>
#include <engine/clock.h>
#include <engine/graphics/renderer.h>
#include <engine/logger.h>
#include <engine/screeneffects.h>

using namespace engine;

graphics::ColorGradient ScreenEffects::LightSource::lightColor()   
{
    return {{1.0f, 1.0f, 1.0f}};
}

ScreenEffects::ScreenEffects(std::shared_ptr<Context>& ctx)
    : ContextObject(ctx)
    , mRadiusMod(2.0f)
{
}

void ScreenEffects::draw(graphics::Renderer& renderer, Clock& clock, Camera& camera, const std::vector<std::shared_ptr<LightSource>> &sources)
{
    uint32_t h = clock.hour();

    // Daylight
    if (clock.daylight()) {
        return;
    }

    float_t baseAlpha = 0.75f;
    switch(clock.month()) {
    case 1:
        baseAlpha += 0.08f;
        break;
    case 3:
        baseAlpha -= 0.08f;
        break;
    }

    graphics::Color color(1.0f - baseAlpha, 1.0f - baseAlpha, 1.0f - baseAlpha);

    // Night
    if (h >= clock.dawn() && h < clock.dusk()) {
        float_t currentHour = clock.currentHour();

        // Sunrise
        if (h >= clock.dawn() && h < clock.sunrise()) {
            float_t diff = (currentHour - clock.dawn() * 60) / ((clock.sunrise() - clock.dawn()) * 60.0f);
            float_t alpha = baseAlpha - baseAlpha * diff;
            float_t d = 0.2f * diff;
            float_t c = 1.0f - alpha;
            color = graphics::Color(std::min(c + d, 1.0f), c, c);
        // Sunset
        } else if (h >= clock.sunset() && h < clock.dusk()) {
            float_t diff = (currentHour - (clock.sunset() * 60)) / ((clock.dusk() - clock.sunset()) * 60.0f);
            float_t alpha = baseAlpha * diff;
            float_t d = 0.2f - 0.2f * diff;
            float_t c = 1.0f - alpha;
            color = graphics::Color(std::min(c + d, 1.0f), c, c);
        }
    }

    Types::Overlay overlay(renderer.width(), renderer.height(), color);
    for (auto& source: sources) {
        if (source->lightRadius() == 0) {
            continue;
        }

        graphics::ColorGradient l = graphics::ColorGradient(source->lightColor(), source->lightStrength());
        overlay.addEllipse(Types::FilledEllipse(source->lightPosition().x - camera.x(), source->lightPosition().y - camera.y(), source->lightRadius(), l));
    }

    renderer.drawOverlay({0, 0}, overlay, mRadiusMod);
}

bool ScreenEffects::processAsync(uint64_t frameDiff)
{
    float_t m = mRadiusMod - (frameDiff / 1000.0f);
    if (m < 0.0f) {
        m = 2.0f;
    }

    bool changed = std::floor(m * 8.0f) != std::floor(mRadiusMod * 8.0f);
    mRadiusMod = m;
    return changed;
}