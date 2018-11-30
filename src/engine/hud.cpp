#include <engine/assetmanager.h>
#include <engine/clock.h>
#include <engine/frametimer.h>
#include <engine/hud.h>
#include <engine/image.h>
#include <engine/renderer.h>

using namespace engine;

Hud::Hud()
{
    mClockImage = AssetManager::get()->image(AssetManager::Ui, "hud_clock");
    mItemEquipped = AssetManager::get()->image(AssetManager::Ui, "hud_item_equipped");
    mTestItem = AssetManager::get()->image(AssetManager::Ui, "item_hammer");
    mBarSmall = AssetManager::get()->image(AssetManager::Ui, "hud_bar_small");
}

void Hud::draw(Renderer& renderer, Clock& clock, FrameTimer& frameTimer)
{
    renderer.save();

    // Bottom hud
    renderer.translate(renderer.width() / 2, renderer.height() - (mClockImage->height() / 2) - 16);

    renderer.translate(-(mClockImage->height() / 2), 0);
    drawItems(renderer);
    renderer.translate((mClockImage->height() / 2), 0);

    renderer.translate((mClockImage->height() / 2), 0);
    drawHealth(renderer);
    renderer.translate(-(mClockImage->height() / 2), 0);

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
    Types::Rect<> textRc(0, 0, clockDimen.width, clockDimen.height);
    renderer.drawText(textRc, clock.timeFormatted(), {0, 0, 0, 175}, 18, Types::TextAlign({Types::TextAlign::CentreH, Types::TextAlign::CentreV}), "hud");

    renderer.translate(centreX, centreY);

    // Foreground
    renderer.translate(-centreX, -centreY);

    clockRc.x = clockDimen.width;
    renderer.drawImage(*mClockImage.get(), {}, clockRc);

    renderer.translate(centreX, centreY);
}

void Hud::drawHealth(Renderer& renderer)
{
    renderer.translate(-10, -(42 / 2));

    // Stamina
    drawBarSmall(renderer, 96, {255, 255, 0, 175}, 100);
    renderer.translate(0, 42);

    // Health
    renderer.translate(0, -(mBarSmall->height()));
    drawBarSmall(renderer, 96, {255, 0, 0, 175}, 50);
    renderer.translate(0, mBarSmall->height());

    renderer.translate(10, -(42 / 2));
}

void Hud::drawItems(Renderer& renderer)
{
    Types::Rect<> leftItemRc(0, 0, mItemEquipped->width() / 2, mItemEquipped->height());
    renderer.translate(-(leftItemRc.width / 2), -(leftItemRc.height / 2));
    renderer.drawImage(*mItemEquipped.get(), {}, leftItemRc);

    renderer.translate(leftItemRc.height / 2, leftItemRc.height / 2);
    renderer.translate(-(mTestItem->width() / 2), -(mTestItem->height() / 2));
    renderer.drawImage(*mTestItem.get());
    renderer.translate(mTestItem->width() / 2, mTestItem->height() / 2);
    renderer.translate(-leftItemRc.height / 2, -leftItemRc.height / 2);

    leftItemRc.x = leftItemRc.width;
    renderer.drawImage(*mItemEquipped.get(), {}, leftItemRc);

    renderer.translate((leftItemRc.width / 2), (leftItemRc.height / 2));
}

void Hud::drawBarSmall(Renderer& renderer, uint32_t width, Types::Color fillColor, uint8_t fillPercent)
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
    renderer.fillRect({0, 0, static_cast<int32_t>(width * (fillPercent / 100.0f)), fillRc.height}, fillColor);
    renderer.translate(-(bgRc.width), 0);

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