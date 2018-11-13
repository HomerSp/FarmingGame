#pragma once

#include <engine/camera.h>
#include <engine/clock.h>
#include <engine/renderer.h>
#include <engine/types.h>

namespace engine {
class ScreenEffects {
public:
    class LightSource {
    public:
        virtual Types::Point<int32_t> light() = 0;
        virtual int32_t radius() = 0;
        virtual float strength() = 0;
    };

public:
    ScreenEffects();

    void draw(Renderer& renderer, Clock& clock, Camera& camera, const std::vector<std::shared_ptr<LightSource>> &sources);
};
}