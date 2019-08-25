#include <engine/camera.h>
#include <engine/clock.h>
#include <engine/context.h>
#include <engine/graphics/renderer.h>
#include <engine/screeneffects.h>
#include <engine/weather.h>

using namespace engine;

ScreenEffects::ScreenEffects(std::shared_ptr<Context>& ctx, graphics::Renderer& renderer, uint32_t lightsCount)
    : ContextObject(ctx)
{
    mOverlay = std::make_unique<Overlay>(renderer, lightsCount);
}

void ScreenEffects::draw(graphics::Renderer& renderer, Clock& clock, Camera& camera, const std::vector<std::shared_ptr<Overlay::LightSource>> &sources, Weather& weather)
{
    uint32_t h = clock.hour();

    auto weatherType = weather.type();
    float_t baseC = 1.0f;
    if (weatherType == Weather::Overcast || weatherType == Weather::Rain) {
        baseC = 0.75f;
    }

    graphics::Color color(baseC, baseC, baseC);
    if (!clock.daylight()) {
        float_t baseAlpha = baseC - 0.25f;
        switch(clock.month()) {
        case 1:
            baseAlpha += 0.08f;
            break;
        case 3:
            baseAlpha -= 0.08f;
            break;
        }

        float_t c = baseC - baseAlpha;

        // Night
        if (h >= clock.dawn() && h < clock.dusk()) {
            float_t currentHour = clock.currentHour();

            // Sunrise
            if (h >= clock.dawn() && h < clock.sunrise()) {
                float_t diff = (currentHour - clock.dawn() * 60) / ((clock.sunrise() - clock.dawn()) * 60.0f);
                c = baseC - (baseAlpha - baseAlpha * diff);
            // Sunset
            } else if (h >= clock.sunset() && h < clock.dusk()) {
                float_t diff = (currentHour - (clock.sunset() * 60)) / ((clock.dusk() - clock.sunset()) * 60.0f);
                c = baseC - (baseAlpha * diff);
            }
        }

        color = graphics::Color(c, c, c);
    }

    mOverlay->setBackground(color);
    mOverlay->draw(renderer, camera, sources);
}

bool ScreenEffects::processAsync(uint64_t frameDiff)
{
    return mOverlay->processAsync(frameDiff);
}