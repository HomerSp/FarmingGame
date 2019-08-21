#include <engine/camera.h>
#include <engine/clock.h>
#include <engine/graphics/renderer.h>
#include <engine/logger.h>
#include <engine/screeneffects.h>

using namespace engine;

ScreenEffects::ScreenEffects(std::shared_ptr<Context>& ctx, graphics::Renderer& renderer, uint32_t lightsCount)
    : ContextObject(ctx)
{
    mOverlay = std::make_unique<Overlay>(renderer, lightsCount);
}

void ScreenEffects::draw(graphics::Renderer& renderer, Clock& clock, Camera& camera, const std::vector<std::shared_ptr<Overlay::LightSource>> &sources)
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

    mOverlay->setBackground(color);
    mOverlay->draw(renderer, camera, sources);
}

bool ScreenEffects::processAsync(uint64_t frameDiff)
{
    return mOverlay->processAsync(frameDiff);
}