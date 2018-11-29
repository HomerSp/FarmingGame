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
    void drawItems(Renderer& renderer);

private:
    std::shared_ptr<Image> mClockImage;
    std::shared_ptr<Image> mLeftItemImage;
    std::shared_ptr<Image> mItemImage;
    std::shared_ptr<Image> mTestItem;
};
}