#pragma once

#include <atomic>

#include <engine/contextobject.h>

namespace engine {

class Clock;
class Context;
class FrameTimer;
class Item;
class Player;

namespace graphics {
class Buffer;
class Image;
class Renderer;
class Texture;
}

class Hud : public ContextObject {
public:
    Hud(std::shared_ptr<Context>& ctx, graphics::Renderer& renderer);

    void draw(graphics::Renderer& renderer, Clock& clock, Player& player, FrameTimer& frameTimer);

    void expandItems(bool expand);

    bool isExpanded() const;

protected:
    void drawClock(graphics::Renderer& renderer, Clock& clock);
    void drawHealth(graphics::Renderer& renderer, Player& player);
    void drawItems(graphics::Renderer& renderer, Player& player);
    void drawExpandedItem(graphics::Renderer& renderer, Item& item);

    void drawBarSmall(graphics::Renderer& renderer, uint32_t width, graphics::Color fillColor, float_t fillPercent, uint32_t indicatorX);

private:
    Types::Point<> mBoxSize;
    uint32_t mHealthStaminaWidth;

    std::unique_ptr<graphics::Image> mClockImage;
    std::unique_ptr<graphics::Image> mHudItemEquipped, mHudItem;
    std::unique_ptr<graphics::Image> mBarSmall;
    std::unique_ptr<graphics::Image> mHealthStamina;
    std::unique_ptr<graphics::Image> mSeasonsImage;

    std::atomic<bool> mExpanded;

    std::unique_ptr<graphics::Buffer> mBuffer;
    std::unique_ptr<graphics::Texture> mTextures;
};
}