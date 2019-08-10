#include <engine/camera.h>
#include <engine/clock.h>
#include <engine/logger.h>
#include <engine/renderer.h>
#include <engine/screeneffects.h>

using namespace engine;

Types::ColorGradient ScreenEffects::LightSource::lightColor()   
{
    return {{255, 255, 255}};
}

ScreenEffects::ScreenEffects(std::shared_ptr<Context>& ctx)
    : ContextObject(ctx)
    , mRadiusMod(2.0f)
{
}

void ScreenEffects::draw(Renderer& renderer, Clock& clock, Camera& camera, const std::vector<std::shared_ptr<LightSource>> &sources)
{
    uint32_t h = clock.hour();

    // Daylight
    if (clock.daylight()) {
        return;
    }

    uint8_t baseAlpha = 200;
    switch(clock.month()) {
    case 1:
        baseAlpha += 20;
        break;
    case 3:
        baseAlpha -= 20;
        break;
    }

    Types::Color color(255 - baseAlpha, 255 - baseAlpha, 255 - baseAlpha);

    // Night
    if (h >= clock.dawn() && h < clock.dusk()) {
        uint32_t currentHour = clock.current() % (24 * 60);

        // Sunrise
        if (h >= clock.dawn() && h < clock.sunrise()) {
            float_t diff = (currentHour - clock.dawn() * 60) / ((clock.sunrise() - clock.dawn()) * 60.0f);
            float_t alpha = baseAlpha - baseAlpha * diff;
            int32_t d = 50 * diff;
            uint8_t c = 255 - std::floor(alpha);
            color = Types::Color(std::min(c + d, 255), c, c);
        // Sunset
        } else if (h >= clock.sunset() && h < clock.dusk()) {
            float_t diff = (currentHour - (clock.sunset() * 60)) / ((clock.dusk() - clock.sunset()) * 60.0f);
            float_t alpha = baseAlpha * diff;
            int32_t d = 50 - 50 * diff;
            uint8_t c = 255 - std::floor(alpha);
            color = Types::Color(std::min(c + d, 255), c, c);
        }
    }

    Types::Overlay overlay(renderer.width(), renderer.height(), color);
    for (auto& source: sources) {
        if (source->lightRadius() == 0) {
            continue;
        }

        Types::ColorGradient l = Types::ColorGradient(source->lightColor(), source->lightStrength());
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