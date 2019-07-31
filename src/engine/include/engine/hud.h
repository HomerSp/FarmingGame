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

    bool isExpanded() const;

protected:
    void drawClock(Renderer& renderer, Clock& clock);
    void drawHealth(Renderer& renderer, Player& player);
    void drawItems(Renderer& renderer, Player& player);
    void drawExpandedItem(Renderer& renderer, Item& item);

    void drawBarSmall(Renderer& renderer, uint32_t width, Types::Color fillColor, float_t fillPercent, uint32_t indicatorX);

private:
    Types::Point<> mBoxSize;
    uint32_t mHealthStaminaWidth;

    std::unique_ptr<Image> mClockImage;
    std::unique_ptr<Image> mHudItemEquipped, mHudItem;
    std::unique_ptr<Image> mBarSmall;
    std::unique_ptr<Image> mHealthStamina;
    std::unique_ptr<Image> mSeasonsImage;

    std::atomic<bool> mExpanded;
};
}