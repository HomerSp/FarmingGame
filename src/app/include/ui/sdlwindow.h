#pragma once

#include <cstdint>
#include <memory>
#include <unordered_map>
#include <SDL3/SDL.h>
#include <engine/engine.h>
#include <engine/keys.h>

class SdlRenderer;

class SdlWindow
{
public:
    SdlWindow();
    ~SdlWindow();

    bool init(uint32_t width, uint32_t height);
    void run();

private:
    void handleEvent(const SDL_Event& event);
    void update(double deltaTime);
    void render();

    SDL_Window* mWindow;
    SDL_GPUDevice* mDevice;
    std::shared_ptr<engine::Engine> mEngine;
    std::shared_ptr<SdlRenderer> mRenderer;
    std::unordered_map<int32_t, engine::Keys::Type> mKeys;
    bool mRunning;
    uint32_t mWidth;
    uint32_t mHeight;
};
