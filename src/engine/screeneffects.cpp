#include <engine/camera.h>
#include <engine/clock.h>
#include <engine/logger.h>
#include <engine/renderer.h>
#include <engine/screeneffects.h>

using namespace engine;

Types::Color ScreenEffects::LightSource::lightColor()
{
    return {0, 0, 0, 0};
}

ScreenEffects::ScreenEffects()
    : mRadiusMod(2.0f)
{
}

void ScreenEffects::draw(Renderer& renderer, Clock& clock, Camera& camera, const std::vector<std::shared_ptr<LightSource>> &sources)
{
    uint32_t h = clock.hour();

    // Daylight
    if (clock.daylight()) {
        return;
    }

    float_t baseAlpha = 200.0f;
    switch(clock.month()) {
    case 1:
        baseAlpha += 10.0f;
        break;
    case 3:
        baseAlpha -= 10.0f;
        break;
    }

    Types::Color color(0, 0, 0, baseAlpha);

    // Night
    if (h >= clock.dawn() && h < clock.dusk()) {
        uint32_t currentHour = clock.current() % (24 * 60);

        // Sunrise
        if (h >= clock.dawn() && h < clock.sunrise()) {
            float_t diff = (currentHour - clock.dawn() * 60) / ((clock.sunrise() - clock.dawn()) * 60.0f);
            float_t alpha = baseAlpha - baseAlpha * diff;
            color.r = 50 * diff;
            color.a = std::floor(alpha);
        // Sunset
        } else if (h >= clock.sunset() && h < clock.dusk()) {
            float_t diff = (currentHour - (clock.sunset() * 60)) / ((clock.dusk() - clock.sunset()) * 60.0f);
            float_t alpha = baseAlpha * diff;
            color.r = 50 - 50 * diff;
            color.a = std::floor(alpha);
        }
    }

    int32_t radiusMod = std::abs(static_cast<int32_t>((mRadiusMod - 1.0f) * 8.0f));

    Types::Overlay overlay(renderer.width(), renderer.height(), color);
    for (auto& source: sources) {
        if (source->lightRadius() == 0) {
            continue;
        }

        Types::Color l = color;
        l.r = l.r * (1.0f - source->lightStrength());
        l.a = l.a * (1.0f - source->lightStrength());

        overlay.addEllipse(Types::FilledEllipse(source->lightPosition().x - camera.x(), source->lightPosition().y - camera.y(), source->lightRadius() + radiusMod, l));
        overlay.addEllipse(Types::FilledEllipse(source->lightPosition().x - camera.x(), source->lightPosition().y - camera.y(), source->lightRadius() + radiusMod, source->lightColor()));
    }

    renderer.drawOverlay({0, 0}, overlay);
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