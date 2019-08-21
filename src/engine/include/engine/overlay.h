#pragma once

#include <mutex>
#include <set>

#include <engine/camera.h>
#include <engine/graphics/buffer.h>
#include <engine/graphics/color.h>
#include <engine/graphics/renderer.h>
#include <engine/types.h>

namespace engine {
class Overlay {
public:
    class LightSource {
    public:
        virtual Types::Point<int32_t> lightPosition() = 0;
        virtual int32_t lightRadius() = 0;
        virtual graphics::ColorGradient lightColor();
    };

public:
    Overlay(graphics::Renderer& renderer, uint32_t lightsCount);

    const graphics::Color& background() const;
    graphics::Buffer& buffer();

    void draw(graphics::Renderer& renderer, Camera& camera, const std::vector<std::shared_ptr<LightSource>> &sources);

    bool processAsync(uint64_t frameDiff);

    void setBackground(const graphics::Color& bg);

private:
    float_t mRadiusMod;
    graphics::Color mBackground;
    std::vector<Types::FilledEllipse> mLights;
    std::unique_ptr<graphics::Buffer> mBuffer;
};
}