#pragma once

#include <engine/types.h>

namespace engine {

class Camera;
class Clock;
class Renderer;

class ScreenEffects {
public:
    class LightSource {
    public:
        virtual Types::Point<int32_t> position() = 0;
        virtual int32_t radius() = 0;
        virtual float strength() = 0;
    };

public:
    ScreenEffects();

    void draw(Renderer& renderer, Clock& clock, Camera& camera, const std::vector<std::shared_ptr<LightSource>> &sources);
};
}