#pragma once

#include <cstdint>
#include <map>
#include <memory>
#include <unordered_map>
#include <string>
#include <vector>
#include <SDL3/SDL.h>
#include <SDL3_ttf/SDL_ttf.h>

#include <engine/engine.h>
#include <engine/graphics/buffer.h>
#include <engine/graphics/image.h>
#include <engine/graphics/matrix.h>
#include <engine/graphics/texture.h>
#include <engine/graphics/renderer.h>
#include <engine/overlay.h>
#include <engine/particles.h>

#include <ui/sdlbuffer.h>
#include <ui/sdltexture.h>

class SdlRenderer : public engine::graphics::Renderer
{
public:
    SdlRenderer(SDL_GPUDevice* device, SDL_Window* window);
    virtual ~SdlRenderer();

    void beginFrame(SDL_GPUCommandBuffer* cmd, SDL_GPUTexture* swapchainTexture);
    void endFrame();

    void setSize(uint32_t w, uint32_t h, double devicePixelRatio) override;
    void paint(std::shared_ptr<engine::Engine>& engine) override;

    int32_t width() override;
    int32_t height() override;

    void fillEllipse(const engine::Types::Rect<>& dst, const engine::graphics::Color& color) override;
    void fillRect(const engine::Types::Rect<>& dst, const engine::graphics::Color& color) override;

    void drawImage(const engine::graphics::Image& img, engine::Types::Rect<> dst, engine::Types::Rect<> src) override;
    void drawText(const engine::Types::Rect<>& dst, const std::string& text, const engine::graphics::Color& color, int32_t size, engine::Types::TextAlign align, std::string type, bool shadow) override;
    void drawOverlay(const engine::Types::Point<>& dst, engine::Overlay& overlay, uint32_t lightsCount, float mod) override;
    void drawParticles(const engine::Types::Point<>& dst, const engine::Particles& particles, bool round) override;
    void drawTextures(const engine::Types::Point<>& dst, engine::graphics::Texture* texture, engine::graphics::Buffer* buffer, uint32_t count) override;
    void drawTexturesAnim(const engine::Types::Point<>& dst, engine::graphics::Texture* texture, engine::graphics::Buffer* buffer, float_t animFrame, uint32_t count) override;

    void rotate(float_t deg) override;
    void translate(int32_t x, int32_t y) override;

    void save() override;
    void restore() override;

    std::unique_ptr<engine::graphics::Buffer> createBuffer(uint32_t size) const override;
    std::unique_ptr<engine::graphics::Texture> createTexture(uint32_t w, uint32_t h, uint32_t layers) const override;

protected:
    void initContext() override;

private:
    SDL_GPUShader* loadShader(const std::string& filename, SDL_GPUShaderStage stage, uint32_t samplerCount, uint32_t uniformBufferCount);
    TTF_Font* getFont(int32_t size);

    SDL_GPUDevice* mDevice;
    SDL_Window* mWindow;
    SDL_GPUCommandBuffer* mCurrentCmd;
    SDL_GPUTexture* mCurrentSwapchainTexture;

    engine::graphics::Matrix mWorldMatrix;
    engine::graphics::Matrix mFBOMatrix;
    std::vector<engine::graphics::Matrix> mMatrixStack;

    SDL_GPUTexture* mFBO;
    SDL_GPUTexture* mDepthTexture;
    std::unique_ptr<SdlBuffer> mQuadVertexBuffer;
    std::unique_ptr<SdlBuffer> mOverlayBuffer;
    
    SDL_GPUSampler* mSampler;

    // Shaders
    SDL_GPUShader* mTexture2DVert;
    SDL_GPUShader* mTexture2DFrag;
    SDL_GPUShader* mTextureAnim2DVert;
    SDL_GPUShader* mTextureAnim2DFrag;
    SDL_GPUShader* mPointLightVert;
    SDL_GPUShader* mPointLightFrag;
    SDL_GPUShader* mColorVert;
    SDL_GPUShader* mColorFrag;
    SDL_GPUShader* mParticle2DVert;
    SDL_GPUShader* mParticle2DFrag;
    SDL_GPUShader* mFBO2DVert;
    SDL_GPUShader* mFBO2DFrag;
    SDL_GPUShader* mText2DVert;

    // Pipelines
    SDL_GPUGraphicsPipeline* mTexture2DPipeline;
    SDL_GPUGraphicsPipeline* mTextureAnim2DPipeline;
    SDL_GPUGraphicsPipeline* mPointLightPipeline;
    SDL_GPUGraphicsPipeline* mColorPipeline;
    SDL_GPUGraphicsPipeline* mParticle2DPipeline;
    SDL_GPUGraphicsPipeline* mFBO2DPipeline;
    SDL_GPUGraphicsPipeline* mText2DPipeline;

    // Font cache
    std::map<int32_t, TTF_Font*> mFonts;
    std::string mFontPath;

    // Image texture cache (keyed by data pointer)
    std::unordered_map<const void*, std::unique_ptr<SdlTexture>> mImageTextureCache;

    uint32_t mWidth;
    uint32_t mHeight;
    double mDevicePixelRatio;
};
