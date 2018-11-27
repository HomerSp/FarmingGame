#include <engine/assetmanager.h>
#include <engine/clock.h>
#include <engine/frametimer.h>
#include <engine/hud.h>
#include <engine/image.h>
#include <engine/renderer.h>

using namespace engine;

Hud::Hud()
{
    mClockBackground = AssetManager::get()->image(AssetManager::Ui, "clock_background");
    mClockForeground = AssetManager::get()->image(AssetManager::Ui, "clock_foreground");
    mClockMiddle = AssetManager::get()->image(AssetManager::Ui, "clock_midpoint");
    mMinuteHand = AssetManager::get()->image(AssetManager::Ui, "clock_hand_down");
    mHourHand = AssetManager::get()->image(AssetManager::Ui, "clock_hand_down_short");
}

void Hud::draw(Renderer& renderer, Clock& clock, FrameTimer& frameTimer)
{
    renderer.save();

    Types::Dimension<> clockDimen(mClockBackground->width(), mClockBackground->width());

    int hudWidth = 24 * 5 + 10;
    int hudHeight = clockDimen.height + 24 + (10 * 3);
    renderer.translate(renderer.width() - hudWidth, 0);

    Types::Rect<> clockRc(0, 0, hudWidth, hudHeight);
    renderer.fillRect(clockRc, Types::Color(255, 255, 255));

    // Clock
    renderer.translate(hudWidth - (clockDimen.width / 2), clockDimen.height / 2);
    drawClock(renderer, clock);
    renderer.translate(-(hudWidth - (clockDimen.width / 2)), -(clockDimen.height / 2));

    // FPS Counter
    renderer.translate(0, 32 + 15);

    std::stringstream fpsStr;
    fpsStr << frameTimer.framesPerSecond() << "fps";
    renderer.drawText({0, 0, clockRc.width, 32 + 15}, fpsStr.str(), {0, 0, 0}, 24, Types::TextAlign({Types::TextAlign::CentreH, Types::TextAlign::CentreV}), "hud");

    renderer.restore();
}

void Hud::drawClock(Renderer& renderer, Clock& clock)
{
    int32_t centreX = std::floor(mClockBackground->width() / 2);
    int32_t centreY = std::floor(mClockBackground->height() / 2);

    // Background
    renderer.translate(-centreX, -centreY);
    renderer.drawImage(*mClockBackground.get());
    renderer.translate(centreX, centreY);

    // Hour and minutes
    renderer.translate(10 - centreX, 24 - centreY);
    renderer.drawText({0, 0, 22, 16}, clock.hourFormatted(), {255, 255, 255, 175}, 16, Types::TextAlign({Types::TextAlign::CentreH, Types::TextAlign::CentreV}), "hud");
    renderer.translate(22, 0);
    renderer.drawText({0, 0, 22, 16}, clock.minuteFormatted(), {255, 255, 255, 175}, 16, Types::TextAlign({Types::TextAlign::CentreH, Types::TextAlign::CentreV}), "hud");
    renderer.translate(-22, 0);
    renderer.translate(-(10 - centreX), -(24 - centreY));

    // Minute arrow
    int16_t minDeg = (clock.minute() * 6) - 180;
    renderer.rotate(minDeg);
    renderer.translate(-std::floor(mMinuteHand->width() / 2), 0);
    renderer.drawImage(*mMinuteHand.get());
    renderer.translate(std::floor(mMinuteHand->width() / 2), 0);
    renderer.rotate(-minDeg);

    // Hour arrow
    int16_t hourDeg = (((clock.hour() % 12 * 60) + clock.minute()) * 0.5f) - 180;
    renderer.rotate(hourDeg);
    renderer.translate(-std::floor(mHourHand->width() / 2), 0);
    renderer.drawImage(*mHourHand.get());
    renderer.translate(std::floor(mHourHand->width() / 2), 0);
    renderer.rotate(-hourDeg);

    // Mid point
    renderer.translate(-std::floor(mClockMiddle->width() / 2), -std::floor(mClockMiddle->height() / 2));
    renderer.drawImage(*mClockMiddle.get());
    renderer.translate(std::floor(mClockMiddle->width() / 2), std::floor(mClockMiddle->height() / 2));

    // Foreground
    renderer.translate(-centreX, -centreY);
    renderer.drawImage(*mClockForeground.get());
    renderer.translate(centreX, centreY);
}