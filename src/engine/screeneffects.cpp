#include <engine/screeneffects.h>

using namespace engine;

ScreenEffects::ScreenEffects()
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

    renderer.save();

    std::vector<Types::Rect<>> erase;
    for (auto& source: sources) {
        Types::Rect<> playerRc(source->light().x - camera.x(), source->light().y - camera.y(), source->radius(), source->radius());
        Types::Color l = color;
        l.r = l.r * (1.0f - source->strength());
        l.a = l.a * (1.0f - source->strength());

        renderer.fillEllipse(playerRc, l, color);
        erase.push_back(playerRc);
    }

    renderer.eraseEllipses(erase);
    renderer.fillRect({0, 0, renderer.width(), renderer.height()}, color);
    renderer.restore();
}
