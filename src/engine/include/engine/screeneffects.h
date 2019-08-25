#pragma once

#include <atomic>

#include <engine/context.h>
#include <engine/overlay.h>
#include <engine/types.h>

namespace engine {

class Camera;
class Clock;

namespace graphics {
class Renderer;
}

class ScreenEffects : public ContextObject {
public:
    ScreenEffects(std::shared_ptr<Context>& ctx, graphics::Renderer& renderer, uint32_t lightsCount);

    void draw(graphics::Renderer& renderer, Clock& clock, Camera& camera, const std::vector<std::shared_ptr<Overlay::LightSource>> &sources, Weather& weather);

    bool processAsync(uint64_t frameDiff);

private:
    std::unique_ptr<Overlay> mOverlay;
};
}