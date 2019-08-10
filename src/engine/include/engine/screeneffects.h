#pragma once

#include <atomic>

#include <engine/context.h>
#include <engine/types.h>

namespace engine {

class Camera;
class Clock;
class Renderer;

class ScreenEffects : public ContextObject {
public:
    class LightSource {
    public:
        virtual Types::Point<int32_t> lightPosition() = 0;
        virtual int32_t lightRadius() = 0;
        virtual float_t lightStrength() = 0;
        virtual Types::ColorGradient lightColor();
    };

public:
    ScreenEffects(std::shared_ptr<Context>& ctx);

    void draw(Renderer& renderer, Clock& clock, Camera& camera, const std::vector<std::shared_ptr<LightSource>> &sources);

    bool processAsync(uint64_t frameDiff);

private:
    std::atomic<float_t> mRadiusMod;
};
}