#pragma once

#include <cstdint>
#include <SDL3/SDL.h>
#include <engine/graphics/image.h>
#include <engine/graphics/texture.h>

class SdlTexture : public engine::graphics::Texture {
public:
    SdlTexture(SDL_GPUDevice* device, uint32_t w, uint32_t h, uint32_t layers);
    virtual ~SdlTexture();

    virtual void setData(const engine::graphics::Image& image, uint32_t layer) override;

    virtual uint32_t width() const override;
    virtual uint32_t height() const override;

    SDL_GPUTexture* getGpuTexture() const { return mGpuTexture; }

private:
    SDL_GPUDevice* mDevice;
    SDL_GPUTexture* mGpuTexture;
    uint32_t mWidth;
    uint32_t mHeight;
};
