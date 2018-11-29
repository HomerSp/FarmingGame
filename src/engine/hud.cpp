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
    mLeftItemImage = AssetManager::get()->image(AssetManager::Ui, "hud_item_left");
    mTestItem = AssetManager::get()->image(AssetManager::Ui, "item_hammer");
}

void Hud::draw(Renderer& renderer, Clock& clock, FrameTimer& frameTimer)
{
    renderer.save();

    // Bottom hud
    renderer.translate(renderer.width() / 2, renderer.height() - static_cast<int32_t>(mClockImage->height() / 2) - 16);

    renderer.translate(-static_cast<int32_t>(mClockImage->height() / 2), 0);
    drawItems(renderer);
    renderer.translate(static_cast<int32_t>(mClockImage->height() / 2), 0);
    
    drawClock(renderer, clock);
    renderer.translate(-(renderer.width() / 2), -(renderer.height() - static_cast<int32_t>(mClockImage->height() / 2) - 16));

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

    int32_t centreX = static_cast<int32_t>(clockDimen.width / 2);
    int32_t centreY = static_cast<int32_t>(clockDimen.height / 2);

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

void Hud::drawItems(Renderer& renderer)
{
    renderer.translate(-static_cast<int32_t>(mLeftItemImage->width() / 6), -static_cast<int32_t>(mLeftItemImage->height() / 2));

    Types::Rect<> leftItemRc(0, 0, mLeftItemImage->width() / 3, mLeftItemImage->height());
    renderer.drawImage(*mLeftItemImage.get(), {}, leftItemRc);

    renderer.translate(static_cast<int32_t>(mLeftItemImage->height() / 2), static_cast<int32_t>(mLeftItemImage->height() / 2));
    renderer.translate(-(static_cast<int32_t>(mTestItem->width() / 2)), -(static_cast<int32_t>(mTestItem->height() / 2)));
    renderer.drawImage(*mTestItem.get());
    renderer.translate((static_cast<int32_t>(mTestItem->width() / 2)), (static_cast<int32_t>(mTestItem->height() / 2)));
    renderer.translate(-static_cast<int32_t>(mLeftItemImage->height() / 2), -static_cast<int32_t>(mLeftItemImage->height() / 2));

    leftItemRc.x = (mLeftItemImage->width() / 3) * 2;
    renderer.drawImage(*mLeftItemImage.get(), {}, leftItemRc);

    renderer.translate(static_cast<int32_t>(mLeftItemImage->width() / 6), static_cast<int32_t>(mLeftItemImage->height() / 2));
}