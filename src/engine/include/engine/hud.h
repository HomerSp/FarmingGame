#pragma once

#include <atomic>

namespace engine {

class Clock;
class FrameTimer;
class Image;
class Item;
class Player;
class Renderer;

class Hud {
public:
    Hud();

    void draw(Renderer& renderer, Clock& clock, Player& player, FrameTimer& frameTimer);

    void expandItems(bool expand);

protected:
    void drawClock(Renderer& renderer, Clock& clock);
    void drawHealth(Renderer& renderer, Player& player);
    void drawItems(Renderer& renderer, Player& player);
    void drawExpandedItem(Renderer& renderer, Item& item);

    void drawBarSmall(Renderer& renderer, uint32_t width, Types::Color fillColor, float_t fillPercent);

private:
    std::shared_ptr<Image> mClockImage;
    std::shared_ptr<Image> mHudItemEquipped, mHudItem;
    std::shared_ptr<Image> mBarSmall;

    std::atomic<bool> mExpanded;
};
}