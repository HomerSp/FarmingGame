#include <engine/assetmanager.h>
#include <engine/clock.h>
#include <engine/context.h>
#include <engine/frametimer.h>
#include <engine/graphics/image.h>
#include <engine/graphics/renderer.h>
#include <engine/graphics/textureloader.h>
#include <engine/graphics/vector.h>
#include <engine/hud.h>
#include <engine/item.h>
#include <engine/player.h>

using namespace engine;

Hud::Hud(std::shared_ptr<Context>& ctx, graphics::Renderer& renderer)
    : ContextObject(ctx)
    , mBoxSize(32, 32)
    , mHealthStaminaWidth(96)
    , mExpanded(false)
    , mClockBackgroundBufferCount(4)
    , mClockForegroundBufferCount(4)
{
    loadClockTextures(renderer);

    auto clockDimension = mClockTextures->dimension(0);
    clockDimension.width /= 2;

    {
        auto writer = mClockBackgroundBuffer->writer();
        // Clock background
        {
            auto dst = engine::graphics::Vector4D(-(clockDimension.width / 2.0f), -(clockDimension.height / 2.0f), clockDimension.width, clockDimension.height);
            writer.append(dst);
            writer.append(engine::graphics::Vector4D(0, 0, clockDimension.width, clockDimension.height));
            writer.append(static_cast<float_t>(0));
        }

        writer.release();
    }

    {
        auto writer = mClockForegroundBuffer->writer();
        {
            auto dst = engine::graphics::Vector4D(-(clockDimension.width / 2.0f), -(clockDimension.height / 2.0f), clockDimension.width, clockDimension.height);
            writer.append(dst);
            writer.append(engine::graphics::Vector4D(clockDimension.width, 0, clockDimension.width, clockDimension.height));
            writer.append(static_cast<float_t>(0));
        }

        {
            auto boxDimension = mClockTextures->dimension(1);
            Types::Point<float_t> boxPoint((boxDimension.width / 4.0f) + 5.0f, (clockDimension.height / 4.0f) + 5.0f);

            auto boxDst = engine::graphics::Vector4D(boxPoint.x, boxPoint.y, boxDimension.width / 2.0f, boxDimension.height);
            writer.append(boxDst);
            writer.append(engine::graphics::Vector4D(0, 0, boxDimension.width / 2.0f, boxDimension.height));
            writer.append(static_cast<float_t>(1));

            auto seasonDimension = mClockTextures->dimension(2);

            auto seasonDst = engine::graphics::Vector4D(boxPoint.x + 4.0f, boxPoint.y + 4.0f, 24.0f, 24.0f);
            writer.append(seasonDst);
            writer.append(engine::graphics::Vector4D(0, 0, seasonDimension.width / 4.0f, seasonDimension.height));
            writer.append(static_cast<float_t>(2));

            auto frameDst = engine::graphics::Vector4D(boxPoint.x, boxPoint.y, boxDimension.width / 2.0f, boxDimension.height);
            writer.append(frameDst);
            writer.append(engine::graphics::Vector4D(boxDimension.width / 2.0f, 0, boxDimension.width / 2.0f, boxDimension.height));
            writer.append(static_cast<float_t>(1));
        }

        writer.release();
    }
}

void Hud::draw(graphics::Renderer& renderer, Clock& clock, Player& player, FrameTimer& frameTimer)
{
    auto clockDimension = mClockTextures->dimension(0);
    clockDimension.width /= 2;

    Types::Point<> dstPoint(renderer.width() - (clockDimension.width / 2) - 16, 16 + (clockDimension.height / 2));
    renderer.drawTextures(dstPoint, mClockTextures.get(), mClockBackgroundBuffer.get(), mClockBackgroundBufferCount);

    graphics::Color textColor(0.17f, 0.08f, 0.019f, 1.0f);

    // Week day
    Types::Rect<> weekDayRc(dstPoint.x - clockDimension.width / 2, dstPoint.y - clockDimension.height / 2 + 13.0f, clockDimension.width, clockDimension.height / 2);
    renderer.drawText(weekDayRc, clock.weekDayFormattedShort() + " " + clock.dayFormatted(), textColor, 18, Types::TextAlign({Types::TextAlign::CentreH, Types::TextAlign::CentreV}), "hud", true);

    // Hour and minutes
    Types::Rect<> timeRc(dstPoint.x - clockDimension.width / 2, dstPoint.y - 13.0f, clockDimension.width, clockDimension.height / 2);
    renderer.drawText(timeRc, clock.timeFormatted(), textColor, 18, Types::TextAlign({Types::TextAlign::CentreH, Types::TextAlign::CentreV}), "hud", true);

#ifdef DEBUG
    Types::Rect<> fpsRc(0, 0, 24 * 6, 24);
    renderer.fillRect(fpsRc, {1.0f, 1.0f, 1.0f});

    std::stringstream fpsStr;
    fpsStr << frameTimer.framesPerSecond() << "fps";
    renderer.drawText(fpsRc, fpsStr.str(), {0, 0, 0}, 24, Types::TextAlign({Types::TextAlign::CentreH, Types::TextAlign::CentreV}), "hud");
#endif

    renderer.drawTextures(dstPoint, mClockTextures.get(), mClockForegroundBuffer.get(), mClockForegroundBufferCount);

    /*renderer.save();

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
    renderer.fillRect(fpsRc, {1.0f, 1.0f, 1.0f});

    std::stringstream fpsStr;
    fpsStr << frameTimer.framesPerSecond() << "fps";
    renderer.drawText({0, 0, fpsRc.width, fpsRc.height}, fpsStr.str(), {0, 0, 0}, 24, Types::TextAlign({Types::TextAlign::CentreH, Types::TextAlign::CentreV}), "hud");

    renderer.restore();*/
}

void Hud::expandItems(bool expand)
{
    mExpanded = expand;
}

bool Hud::isExpanded() const {
    return mExpanded;
}

void Hud::drawClock(graphics::Renderer& renderer, Clock& clock)
{
    /*Types::Dimension<> clockDimen(mClockImage->width() / 2, mClockImage->height());

    int32_t centreX = (clockDimen.width / 2);
    int32_t centreY = (clockDimen.height / 2);

    renderer.translate(-centreX, -centreY);

    // Background
    Types::Rect<> clockRc(0, 0, clockDimen.width, clockDimen.height);
    renderer.drawImage(*mClockImage, {}, clockRc);

    // Hour and minutes
    Types::Rect<> timeRc(0, 0, clockDimen.width, clockDimen.height);
    renderer.drawText(timeRc, clock.timeFormatted(), {0, 0, 0, 0.7f}, 18, Types::TextAlign({Types::TextAlign::CentreH, Types::TextAlign::CentreV}), "hud");

    renderer.translate(centreX, centreY);

    // Week day
    renderer.translate(-centreX, -centreY);

    Types::Rect<> weekDayRc(0, 0, clockDimen.width, clockDimen.height / 2);
    renderer.drawText(weekDayRc, clock.weekDayFormattedShort() + "\n" + clock.dayFormatted(), {0, 0, 0, 0.7f}, 18, Types::TextAlign({Types::TextAlign::CentreH, Types::TextAlign::CentreV}), "hud");

    renderer.translate(centreX, centreY);

    // Season
    Types::Rect<> seasonsRect(0, 0, mSeasonsImage->width() / 4, mSeasonsImage->height());
    renderer.translate(-(16/2), (clockRc.height / 4) - (16 / 2));
    renderer.drawImage(*mSeasonsImage, {0, 0, 16, 16}, seasonsRect);
    renderer.translate((16 / 2), -(clockRc.height / 4) + (16 / 2));

    // Foreground
    renderer.translate(-centreX, -centreY);

    clockRc.x = clockDimen.width;
    renderer.drawImage(*mClockImage, {}, clockRc);

    renderer.translate(centreX, centreY);*/
}

void Hud::drawHealth(graphics::Renderer& renderer, Player& player)
{
    /*renderer.translate(-10, -(50 / 2));

    // Stamina
    float_t p = player.stamina() / static_cast<float_t>(player.maxStamina());
    drawBarSmall(renderer, mHealthStaminaWidth, {1.0f, 1.0f, 0.0f, 0.8f}, p, 0);
    renderer.translate(0, 50);

    // Health
    renderer.translate(0, -(mBarSmall->height()));
    p = player.health() / static_cast<float_t>(player.maxHealth());
    drawBarSmall(renderer, mHealthStaminaWidth, {1.0f, 0, 0, 0.8f}, p, mHealthStamina->width() / 2);
    renderer.translate(0, mBarSmall->height());

    renderer.translate(10, -(50 / 2));*/
}

void Hud::drawItems(graphics::Renderer& renderer, Player& player)
{
    /*Types::Rect<> leftItemRc(0, 0, mHudItemEquipped->width() / 2, mHudItemEquipped->height());
    renderer.translate(-(leftItemRc.width / 2), -(leftItemRc.height / 2));
    renderer.drawImage(*mHudItemEquipped, {}, leftItemRc);

    renderer.translate(leftItemRc.height / 2, leftItemRc.height / 2);

    const auto& itemImage = player.currentItem().uiImage();

    renderer.translate(-(mBoxSize.x / 2), -(mBoxSize.y / 2));
    renderer.drawImage(itemImage, {0, 0, mBoxSize.x, mBoxSize.y});
    renderer.translate(mBoxSize.x / 2, mBoxSize.y / 2);
    renderer.translate(-leftItemRc.height / 2, -leftItemRc.height / 2);

    leftItemRc.x = leftItemRc.width;
    renderer.drawImage(*mHudItemEquipped, {}, leftItemRc);

    renderer.translate((leftItemRc.width / 2), (leftItemRc.height / 2));

    if (mExpanded) {
        renderer.translate(-(leftItemRc.width / 2), -(leftItemRc.height / 2));

        Types::Rect<> itemRc(0, 0, mHudItem->width() / 2, mHudItem->height());
        renderer.translate(-(itemRc.width - 5) * 10, 0);

        for (uint8_t i = 0; i < 10; i++) {
            renderer.drawImage(*mHudItem, {}, itemRc);

            if (i == player.currentItemIndex()) {
                renderer.fillRect(itemRc, {1.0f, 1.0f, 0.0f, 0.7f});
            }

            if (player.hasItem(i)) {
                const auto &itemsImage = player.item(i).uiImage();
                renderer.translate((itemRc.width / 2), (itemRc.height / 2));
                renderer.translate(-(mBoxSize.x / 2), -(mBoxSize.y / 2));
                renderer.drawImage(itemsImage, {0, 0, mBoxSize.x, mBoxSize.y});
                renderer.translate((mBoxSize.x / 2), (mBoxSize.y / 2));
                renderer.translate(-(itemRc.width / 2), -(itemRc.height / 2));
            }

            renderer.drawImage(*mHudItem, {}, {itemRc.x + itemRc.width, itemRc.y, itemRc.width, itemRc.height});

            renderer.translate(itemRc.width - 5, 0);
        }

        renderer.translate((leftItemRc.width / 2), (leftItemRc.height / 2));
    }*/
}

void Hud::drawBarSmall(graphics::Renderer& renderer, uint32_t width, graphics::Color fillColor, float_t fillPercent, uint32_t indicatorX)
{
    /*Types::Rect<int32_t> bgRc(0, 0, (mBarSmall->width() / 2) / 5, mBarSmall->height());
    Types::Rect<int32_t> fillRc(0, 0, std::max(0, static_cast<int32_t>(width - (bgRc.width * 2))), bgRc.height);

    // Background
    renderer.drawImage(*mBarSmall, {}, bgRc);

    bgRc.x += bgRc.width;
    renderer.translate(bgRc.width, 0);
    renderer.drawImage(*mBarSmall, {}, bgRc);

    bgRc.x += bgRc.width;
    renderer.translate(bgRc.width, 0);
    renderer.drawImage(*mBarSmall, fillRc, bgRc);

    bgRc.x += bgRc.width;
    renderer.translate(fillRc.width, 0);
    renderer.drawImage(*mBarSmall, {}, bgRc);

    bgRc.x += bgRc.width;
    renderer.translate(bgRc.width, 0);
    renderer.drawImage(*mBarSmall, {}, bgRc);

    renderer.translate(-(fillRc.width + (bgRc.width * 3)), 0);

    // Fill
    renderer.translate(bgRc.width, 0);
    renderer.fillRect({0, 0, static_cast<int32_t>(width * fillPercent), fillRc.height}, fillColor);
    renderer.translate(-(bgRc.width), 0);

    Types::Rect<> healthStaminaRc(indicatorX, 0, mHealthStamina->width() / 2, mHealthStamina->height());

    renderer.translate(5 + mHealthStaminaWidth - healthStaminaRc.width, 5);
    renderer.drawImage(*mHealthStamina, {}, healthStaminaRc);
    renderer.translate(-(5 + mHealthStaminaWidth - healthStaminaRc.width), -5);

    // Foreground
    bgRc.x = bgRc.width * 5;
    renderer.drawImage(*mBarSmall, {}, bgRc);

    bgRc.x += bgRc.width;
    renderer.translate(bgRc.width, 0);
    renderer.drawImage(*mBarSmall, {}, bgRc);

    bgRc.x += bgRc.width;
    renderer.translate(bgRc.width, 0);
    renderer.drawImage(*mBarSmall, fillRc, bgRc);

    bgRc.x += bgRc.width;
    renderer.translate(fillRc.width, 0);
    renderer.drawImage(*mBarSmall, {}, bgRc);

    bgRc.x += bgRc.width;
    renderer.translate(bgRc.width, 0);
    renderer.drawImage(*mBarSmall, {}, bgRc);

    renderer.translate(-(fillRc.width + (bgRc.width * 3)), 0);*/
}

void Hud::loadClockTextures(graphics::Renderer& renderer)
{
    std::vector<std::string> imgs;
    imgs.emplace_back("hud_round_big");
    imgs.emplace_back("hud_round_small");
    imgs.emplace_back("seasons");

    graphics::TextureLoader textureLoader(renderer);
    for (auto& i: imgs) {
        textureLoader += *context().assetManager().image(AssetManager::Ui, i);
    }

    textureLoader.finish(mClockTextures);

    uint32_t bufferSize = engine::graphics::Vector4D::Size() * 2 + sizeof(float_t);
    mClockBackgroundBuffer = renderer.createBuffer(bufferSize * mClockBackgroundBufferCount);
    mClockForegroundBuffer = renderer.createBuffer(bufferSize * mClockForegroundBufferCount);
}

void Hud::loadItemTextures(graphics::Renderer& renderer)
{

}
