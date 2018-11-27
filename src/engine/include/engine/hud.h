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

private:
    std::shared_ptr<Image> mClockBackground, mClockForeground;
    std::shared_ptr<Image> mClockMiddle;
    std::shared_ptr<Image> mMinuteHand, mHourHand;
};
}