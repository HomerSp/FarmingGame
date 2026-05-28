#include <iostream>
#include <ui/sdlrenderer.h>
#include <ui/sdlwindow.h>

SdlWindow::SdlWindow()
    : mWindow(nullptr)
    , mDevice(nullptr)
    , mRunning(false)
    , mWidth(1280)
    , mHeight(720)
{
}

SdlWindow::~SdlWindow()
{
    mEngine.reset();
    mRenderer.reset();

    if (mDevice) {
        SDL_DestroyGPUDevice(mDevice);
    }
    if (mWindow) {
        SDL_DestroyWindow(mWindow);
    }
}

bool SdlWindow::init(uint32_t width, uint32_t height)
{
    mWidth = width;
    mHeight = height;

    if (!SDL_Init(SDL_INIT_VIDEO)) {
        std::cerr << "SDL_Init failed: " << SDL_GetError() << std::endl;
        return false;
    }

    if (!TTF_Init()) {
        std::cerr << "TTF_Init failed: " << SDL_GetError() << std::endl;
        return false;
    }

    mWindow = SDL_CreateWindow("Farming", width, height, SDL_WINDOW_RESIZABLE);
    if (!mWindow) {
        std::cerr << "SDL_CreateWindow failed: " << SDL_GetError() << std::endl;
        return false;
    }

    // Create GPU Device (SPIR-V shader format is required for Vulkan)
    mDevice = SDL_CreateGPUDevice(SDL_GPU_SHADERFORMAT_SPIRV, true, nullptr);
    if (!mDevice) {
        std::cerr << "SDL_CreateGPUDevice failed: " << SDL_GetError() << std::endl;
        return false;
    }

    if (!SDL_ClaimWindowForGPUDevice(mDevice, mWindow)) {
        std::cerr << "SDL_ClaimWindowForGPUDevice failed: " << SDL_GetError() << std::endl;
        return false;
    }

    mRenderer = std::make_shared<SdlRenderer>(mDevice, mWindow);
    mEngine = std::make_shared<engine::Engine>(width, height, mRenderer);

    // Setup keys
    mKeys[SDLK_UP] = engine::Keys::Up;
    mKeys[SDLK_DOWN] = engine::Keys::Down;
    mKeys[SDLK_LEFT] = engine::Keys::Left;
    mKeys[SDLK_RIGHT] = engine::Keys::Right;
    mKeys[SDLK_LSHIFT] = engine::Keys::Run;
    mKeys[SDLK_RSHIFT] = engine::Keys::Run;
    mKeys[SDLK_E] = engine::Keys::Use;
    mKeys[SDLK_Q] = engine::Keys::ExpandHudItems;
    mKeys[SDLK_P] = engine::Keys::TestPause;
    mKeys[SDLK_F] = engine::Keys::TestFriction;
    mKeys[SDLK_S] = engine::Keys::TestSlowMode;
    mKeys[SDLK_T] = engine::Keys::TestFastForward;

    mEngine->setKeyMap(mKeys);
    
    // Note: devicePixelRatio in SDL3 can be queried from window, but 1.0 is standard on Linux/X11
    float pixelRatio = 1.0f;
    SDL_GetWindowSizeInPixels(mWindow, (int*)&width, (int*)&height);
    mEngine->setSize(width, height, pixelRatio);

    mRunning = true;
    return true;
}

void SdlWindow::run()
{
    while (mRunning) {
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            handleEvent(event);
        }

        if (!mRunning) break;

        mEngine->process();
        render();

        // Prevent 100% CPU usage by doing a tiny sleep (e.g. 1ms)
        SDL_Delay(1);
    }
}

void SdlWindow::handleEvent(const SDL_Event& event)
{
    switch (event.type) {
    case SDL_EVENT_QUIT:
        mRunning = false;
        break;
    case SDL_EVENT_WINDOW_RESIZED: {
        int w = event.window.data1;
        int h = event.window.data2;
        mEngine->setSize(w, h, 1.0f);
        break;
    }
    case SDL_EVENT_WINDOW_FOCUS_GAINED:
        mEngine->setFocus(true);
        break;
    case SDL_EVENT_WINDOW_FOCUS_LOST:
        mEngine->setFocus(false);
        break;
    case SDL_EVENT_KEY_DOWN:
        if (!event.key.repeat) {
            mEngine->setKeyDown(event.key.key);
        }
        break;
    case SDL_EVENT_KEY_UP:
        if (!event.key.repeat) {
            mEngine->setKeyUp(event.key.key);
        }
        break;
    }
}

void SdlWindow::render()
{
    SDL_GPUCommandBuffer* cmd = SDL_AcquireGPUCommandBuffer(mDevice);
    if (!cmd) return;

    SDL_GPUTexture* swapchainTexture = nullptr;
    if (SDL_AcquireGPUSwapchainTexture(cmd, mWindow, &swapchainTexture, nullptr, nullptr) && swapchainTexture) {
        mRenderer->beginFrame(cmd, swapchainTexture);
        mRenderer->paint(mEngine);
        mRenderer->endFrame();
    } else {
        SDL_SubmitGPUCommandBuffer(cmd);
    }
}
