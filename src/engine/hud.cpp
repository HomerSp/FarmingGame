#include <engine/assetmanager.h>
#include <engine/character/character.h>
#include <engine/clock.h>
#include <engine/frametimer.h>
#include <engine/hud.h>
#include <engine/image.h>
#include <engine/item.h>
#include <engine/player.h>
#include <engine/renderer.h>

using namespace engine;

Hud::Hud()
    : mBoxSize(32, 32)
    , mHealthStaminaWidth(96)
    , mExpanded(false)
{
    mClockImage = AssetManager::get()->image(AssetManager::Ui, "hud_clock");
    mHudItemEquipped = AssetManager::get()->image(AssetManager::Ui, "hud_bar_left");
    mHudItem = AssetManager::get()->image(AssetManager::Ui, "hud_item");
    mBarSmall = AssetManager::get()->image(AssetManager::Ui, "hud_bar_small");
    mHealthStamina = AssetManager::get()->image(AssetManager::Ui, "hud_health_stamina");
    mSeasonsImage = AssetManager::get()->image(AssetManager::Ui, "seasons");
}

void Hud::draw(Renderer& renderer, Clock& clock, Player& player, FrameTimer& frameTimer)
{
    renderer.save();

    // Bottom hud
    renderer.translate(renderer.width() / 2, renderer.height() - (mClockImage->height() / 2) - 16);

    renderer.translate(-(mClockImage->width() / 4), 0);
    drawItems(renderer, player);
    renderer.translate((mClockImage->width() / 4), 0);

    renderer.translate((mClockImage->width() / 4), 0);
    drawHealth(renderer, player);
    renderer.translate(-(mClockImage->width() / 4), 0);

    drawClock(renderer, clock);
    renderer.translate(-(renderer.width() / 2), -(renderer.height() - (mClockImage->height() / 2) - 16));

    // FPS Counter
    Types::Rect<> fpsRc(0, 0, 24 * 6, 24);
    renderer.translate(renderer.width() - fpsRc.width, 0);
    renderer.fillRect(fpsRc, Types::Color(255, 255, 255));

    std::stringstream fpsStr;
    fpsStr << frameTimer.framesPerSecond() << "fps";
    renderer.drawText({0, 0, fpsRc.width, fpsRc.height}, fpsStr.str(), {0, 0, 0}, 24, Types::TextAlign({Types::TextAlign::CentreH, Types::TextAlign::CentreV}), "hud");

    renderer.restore();
}

void Hud::expandItems(bool expand)
{
    mExpanded = expand;
}

bool Hud::isExpanded() const {
    return mExpanded;
}

void Hud::drawClock(Renderer& renderer, Clock& clock)
{
    Types::Dimension<> clockDimen(mClockImage->width() / 2, mClockImage->height());

    int32_t centreX = (clockDimen.width / 2);
    int32_t centreY = (clockDimen.height / 2);

    renderer.translate(-centreX, -centreY);

    // Background
    Types::Rect<> clockRc(0, 0, clockDimen.width, clockDimen.height);
    renderer.drawImage(*mClockImage.get(), {}, clockRc);

    // Hour and minutes
    Types::Rect<> timeRc(0, 0, clockDimen.width, clockDimen.height);
    renderer.drawText(timeRc, clock.timeFormatted(), {0, 0, 0, 175}, 18, Types::TextAlign({Types::TextAlign::CentreH, Types::TextAlign::CentreV}), "hud");

    renderer.translate(centreX, centreY);

    // Week day
    renderer.translate(-centreX, -centreY);

    Types::Rect<> weekDayRc(0, 0, clockDimen.width, clockDimen.height / 2);
    renderer.drawText(weekDayRc, clock.weekDayFormattedShort() + "\n" + clock.dayFormatted(), {0, 0, 0, 175}, 18, Types::TextAlign({Types::TextAlign::CentreH, Types::TextAlign::CentreV}), "hud");

    renderer.translate(centreX, centreY);

    // Season
    Types::Rect<> seasonsRect(0, 0, mSeasonsImage->width() / 4, mSeasonsImage->height());
    renderer.translate(-(16/2), (clockRc.height / 4) - (16 / 2));
    renderer.drawImage(*mSeasonsImage.get(), {0, 0, 16, 16}, seasonsRect);
    renderer.translate((16 / 2), -(clockRc.height / 4) + (16 / 2));

    // Foreground
    renderer.translate(-centreX, -centreY);

    clockRc.x = clockDimen.width;
    renderer.drawImage(*mClockImage.get(), {}, clockRc);

    renderer.translate(centreX, centreY);
}

void Hud::drawHealth(Renderer& renderer, Player& player)
{
    renderer.translate(-10, -(50 / 2));

    // Stamina
    float_t p = player.stamina() / static_cast<float_t>(player.maxStamina());
    drawBarSmall(renderer, mHealthStaminaWidth, {255, 255, 0, 200}, p, 0);
    renderer.translate(0, 50);

    // Health
    renderer.translate(0, -(mBarSmall->height()));
    p = player.health() / static_cast<float_t>(player.maxHealth());
    drawBarSmall(renderer, mHealthStaminaWidth, {255, 0, 0, 200}, p, mHealthStamina->width() / 2);
    renderer.translate(0, mBarSmall->height());

    renderer.translate(10, -(50 / 2));
}

void Hud::drawItems(Renderer& renderer, Player& player)
{
    Types::Rect<> leftItemRc(0, 0, mHudItemEquipped->width() / 2, mHudItemEquipped->height());
    renderer.translate(-(leftItemRc.width / 2), -(leftItemRc.height / 2));
    renderer.drawImage(*mHudItemEquipped.get(), {}, leftItemRc);

    renderer.translate(leftItemRc.height / 2, leftItemRc.height / 2);

    std::shared_ptr<Image> itemImage = player.currentItem()->uiImage();

    renderer.translate(-(mBoxSize.x / 2), -(mBoxSize.y / 2));
    renderer.drawImage(*itemImage.get(), {0, 0, mBoxSize.x, mBoxSize.y});
    renderer.translate(mBoxSize.x / 2, mBoxSize.y / 2);
    renderer.translate(-leftItemRc.height / 2, -leftItemRc.height / 2);

    leftItemRc.x = leftItemRc.width;
    renderer.drawImage(*mHudItemEquipped.get(), {}, leftItemRc);

    renderer.translate((leftItemRc.width / 2), (leftItemRc.height / 2));

    if (mExpanded) {
        renderer.translate(-(leftItemRc.width / 2), -(leftItemRc.height / 2));

        Types::Rect<> itemRc(0, 0, mHudItem->width() / 2, mHudItem->height());
        renderer.translate(-(itemRc.width - 5) * 10, 0);

        const auto& items = player.items();
        for (uint8_t i = 0; i < 10; i++) {
            renderer.drawImage(*mHudItem.get(), {}, itemRc);

            if (i == player.currentItemIndex()) {
                renderer.fillRect(itemRc, {255, 255, 0, 175});
            }

            if (items.find(i) != items.end()) {
                auto itemImage = items.at(i)->uiImage();
                renderer.translate((itemRc.width / 2), (itemRc.height / 2));
                renderer.translate(-(mBoxSize.x / 2), -(mBoxSize.y / 2));
                renderer.drawImage(*itemImage.get(), {0, 0, mBoxSize.x, mBoxSize.y});
                renderer.translate((mBoxSize.x / 2), (mBoxSize.y / 2));
                renderer.translate(-(itemRc.width / 2), -(itemRc.height / 2));
            }

            renderer.drawImage(*mHudItem.get(), {}, {itemRc.x + itemRc.width, itemRc.y, itemRc.width, itemRc.height});

            renderer.translate(itemRc.width - 5, 0);
        }

        renderer.translate((leftItemRc.width / 2), (leftItemRc.height / 2));
    }
}

void Hud::drawBarSmall(Renderer& renderer, uint32_t width, Types::Color fillColor, float_t fillPercent, uint32_t indicatorX)
{
    Types::Rect<int32_t> bgRc(0, 0, (mBarSmall->width() / 2) / 5, mBarSmall->height());
    Types::Rect<int32_t> fillRc(0, 0, std::max(0, static_cast<int32_t>(width - (bgRc.width * 2))), bgRc.height);

    // Background
    renderer.drawImage(*mBarSmall.get(), {}, bgRc);

    bgRc.x += bgRc.width;
    renderer.translate(bgRc.width, 0);
    renderer.drawImage(*mBarSmall.get(), {}, bgRc);

    bgRc.x += bgRc.width;
    renderer.translate(bgRc.width, 0);
    renderer.drawImage(*mBarSmall.get(), fillRc, bgRc);

    bgRc.x += bgRc.width;
    renderer.translate(fillRc.width, 0);
    renderer.drawImage(*mBarSmall.get(), {}, bgRc);

    bgRc.x += bgRc.width;
    renderer.translate(bgRc.width, 0);
    renderer.drawImage(*mBarSmall.get(), {}, bgRc);

    renderer.translate(-(fillRc.width + (bgRc.width * 3)), 0);

    // Fill
    renderer.translate(bgRc.width, 0);
    renderer.fillRect({0, 0, static_cast<int32_t>(width * fillPercent), fillRc.height}, fillColor);
    renderer.translate(-(bgRc.width), 0);

    Types::Rect<> healthStaminaRc(indicatorX, 0, mHealthStamina->width() / 2, mHealthStamina->height());

    renderer.translate(5 + mHealthStaminaWidth - healthStaminaRc.width, 5);
    renderer.drawImage(*mHealthStamina.get(), {}, healthStaminaRc);
    renderer.translate(-(5 + mHealthStaminaWidth - healthStaminaRc.width), -5);

    // Foreground
    bgRc.x = bgRc.width * 5;
    renderer.drawImage(*mBarSmall.get(), {}, bgRc);

    bgRc.x += bgRc.width;
    renderer.translate(bgRc.width, 0);
    renderer.drawImage(*mBarSmall.get(), {}, bgRc);

    bgRc.x += bgRc.width;
    renderer.translate(bgRc.width, 0);
    renderer.drawImage(*mBarSmall.get(), fillRc, bgRc);

    bgRc.x += bgRc.width;
    renderer.translate(fillRc.width, 0);
    renderer.drawImage(*mBarSmall.get(), {}, bgRc);

    bgRc.x += bgRc.width;
    renderer.translate(bgRc.width, 0);
    renderer.drawImage(*mBarSmall.get(), {}, bgRc);

    renderer.translate(-(fillRc.width + (bgRc.width * 3)), 0);
}