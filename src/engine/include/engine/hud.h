#pragma once

namespace engine {

class Clock;
class FrameTimer;
class Image;
class Renderer;

class Hud {
public:
    Hud();

    void draw(Renderer& renderer, Clock& clock, FrameTimer& frameTimer);

protected:
    void drawClock(Renderer& renderer, Clock& clock);
    void drawHealth(Renderer& renderer);
    void drawItems(Renderer& renderer);

    void drawBarSmall(Renderer& renderer, uint32_t width, Types::Color fillColor, uint8_t fillPercent);

private:
    std::shared_ptr<Image> mClockImage;

    std::shared_ptr<Image> mItemEquipped;
    std::shared_ptr<Image> mTestItem;

    std::shared_ptr<Image> mBarSmall;
};
}