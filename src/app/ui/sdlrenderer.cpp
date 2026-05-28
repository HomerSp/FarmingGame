#include <fstream>
#include <iostream>
#include <ui/sdlrenderer.h>
#include <ui/sdltexture.h>
#include <ui/sdlbuffer.h>
#include <engine/context.h>
#include <engine/graphics/vector.h>

using namespace engine;
using namespace engine::graphics;

SdlRenderer::SdlRenderer(SDL_GPUDevice* device, SDL_Window* window)
    : mDevice(device)
    , mWindow(window)
    , mCurrentCmd(nullptr)
    , mCurrentSwapchainTexture(nullptr)
    , mFBO(nullptr)
    , mDepthTexture(nullptr)
    , mSampler(nullptr)
    , mWidth(1280)
    , mHeight(720)
    , mDevicePixelRatio(1.0)
{
    mWorldMatrix.reset();
    mFBOMatrix.reset();

    mQuadVertexBuffer = std::make_unique<SdlBuffer>(mDevice, Quad2D::Size());
    mQuadVertexBuffer->writer()
        .append(Quad2D().lt(0, 0).lb(0, 1).rt(1, 0).rb(1, 1))
        .release();
}

SdlRenderer::~SdlRenderer()
{
    for (auto& pair : mFonts) {
        TTF_CloseFont(pair.second);
    }
    TTF_Quit();

    if (mSampler) SDL_ReleaseGPUSampler(mDevice, mSampler);
    if (mFBO) SDL_ReleaseGPUTexture(mDevice, mFBO);
    if (mDepthTexture) SDL_ReleaseGPUTexture(mDevice, mDepthTexture);

    // Release shaders
    if (mTexture2DVert) SDL_ReleaseGPUShader(mDevice, mTexture2DVert);
    if (mTexture2DFrag) SDL_ReleaseGPUShader(mDevice, mTexture2DFrag);
    if (mTextureAnim2DVert) SDL_ReleaseGPUShader(mDevice, mTextureAnim2DVert);
    if (mTextureAnim2DFrag) SDL_ReleaseGPUShader(mDevice, mTextureAnim2DFrag);
    if (mPointLightVert) SDL_ReleaseGPUShader(mDevice, mPointLightVert);
    if (mPointLightFrag) SDL_ReleaseGPUShader(mDevice, mPointLightFrag);
    if (mColorVert) SDL_ReleaseGPUShader(mDevice, mColorVert);
    if (mColorFrag) SDL_ReleaseGPUShader(mDevice, mColorFrag);
    if (mParticle2DVert) SDL_ReleaseGPUShader(mDevice, mParticle2DVert);
    if (mParticle2DFrag) SDL_ReleaseGPUShader(mDevice, mParticle2DFrag);
    if (mFBO2DVert) SDL_ReleaseGPUShader(mDevice, mFBO2DVert);
    if (mFBO2DFrag) SDL_ReleaseGPUShader(mDevice, mFBO2DFrag);

    // Release pipelines
    if (mTexture2DPipeline) SDL_ReleaseGPUGraphicsPipeline(mDevice, mTexture2DPipeline);
    if (mTextureAnim2DPipeline) SDL_ReleaseGPUGraphicsPipeline(mDevice, mTextureAnim2DPipeline);
    if (mPointLightPipeline) SDL_ReleaseGPUGraphicsPipeline(mDevice, mPointLightPipeline);
    if (mColorPipeline) SDL_ReleaseGPUGraphicsPipeline(mDevice, mColorPipeline);
    if (mParticle2DPipeline) SDL_ReleaseGPUGraphicsPipeline(mDevice, mParticle2DPipeline);
    if (mFBO2DPipeline) SDL_ReleaseGPUGraphicsPipeline(mDevice, mFBO2DPipeline);
}

void SdlRenderer::beginFrame(SDL_GPUCommandBuffer* cmd, SDL_GPUTexture* swapchainTexture)
{
    mCurrentCmd = cmd;
    mCurrentSwapchainTexture = swapchainTexture;

    // Clear swapchain color target to black at start of frame
    SDL_GPUColorTargetInfo targetInfo{};
    targetInfo.texture = swapchainTexture;
    targetInfo.clear_color = SDL_FColor{0.0f, 0.0f, 0.0f, 1.0f};
    targetInfo.load_op = SDL_GPU_LOADOP_CLEAR;
    targetInfo.store_op = SDL_GPU_STOREOP_STORE;

    SDL_GPURenderPass* pass = SDL_BeginGPURenderPass(cmd, &targetInfo, 1, nullptr);
    if (pass) {
        SDL_EndGPURenderPass(pass);
    }

    // Clear depth buffer
    SDL_GPUDepthStencilTargetInfo depthClear{};
    depthClear.texture = mDepthTexture;
    depthClear.clear_depth = 1.0f;
    depthClear.load_op = SDL_GPU_LOADOP_CLEAR;
    depthClear.store_op = SDL_GPU_STOREOP_STORE;
    SDL_GPURenderPass* depthPass = SDL_BeginGPURenderPass(cmd, nullptr, 0, &depthClear);
    if (depthPass) {
        SDL_EndGPURenderPass(depthPass);
    }
}

void SdlRenderer::endFrame()
{
    SDL_SubmitGPUCommandBuffer(mCurrentCmd);
    mCurrentCmd = nullptr;
    mCurrentSwapchainTexture = nullptr;
}

void SdlRenderer::setSize(uint32_t w, uint32_t h, double devicePixelRatio)
{
    mWidth = w;
    mHeight = h;
    mDevicePixelRatio = devicePixelRatio;

    mWorldMatrix.reset();
    mWorldMatrix.ortho(0, w, 0, h, -1, 1); // match OpenGL convention

    mFBOMatrix.reset();
    mFBOMatrix.scale(w, h);

    if (mFBO) {
        SDL_ReleaseGPUTexture(mDevice, mFBO);
    }

    SDL_GPUTextureCreateInfo fboInfo{};
    fboInfo.type = SDL_GPU_TEXTURETYPE_2D;
    fboInfo.format = SDL_GPU_TEXTUREFORMAT_R8G8B8A8_UNORM;
    fboInfo.usage = SDL_GPU_TEXTUREUSAGE_SAMPLER | SDL_GPU_TEXTUREUSAGE_COLOR_TARGET;
    fboInfo.width = w;
    fboInfo.height = h;
    fboInfo.layer_count_or_depth = 1;
    fboInfo.num_levels = 1;
    mFBO = SDL_CreateGPUTexture(mDevice, &fboInfo);

    // Depth texture for z-ordering
    if (mDepthTexture) {
        SDL_ReleaseGPUTexture(mDevice, mDepthTexture);
    }
    SDL_GPUTextureCreateInfo depthInfo{};
    depthInfo.type = SDL_GPU_TEXTURETYPE_2D;
    depthInfo.format = SDL_GPU_TEXTUREFORMAT_D16_UNORM;
    depthInfo.usage = SDL_GPU_TEXTUREUSAGE_DEPTH_STENCIL_TARGET;
    depthInfo.width = w;
    depthInfo.height = h;
    depthInfo.layer_count_or_depth = 1;
    depthInfo.num_levels = 1;
    mDepthTexture = SDL_CreateGPUTexture(mDevice, &depthInfo);

    mOverlayBuffer = std::make_unique<SdlBuffer>(mDevice, Vector4D::Size());
    mOverlayBuffer->writer()
        .append(Vector4D(0, 0, w, h))
        .release();
}

void SdlRenderer::paint(std::shared_ptr<engine::Engine>& engine)
{
    engine->paint();
}

int32_t SdlRenderer::width()
{
    return mWidth;
}

int32_t SdlRenderer::height()
{
    return mHeight;
}

void SdlRenderer::fillEllipse(const engine::Types::Rect<>& dst, const engine::graphics::Color& color)
{
    fillRect(dst, color);
}

void SdlRenderer::fillRect(const engine::Types::Rect<>& dst, const engine::graphics::Color& color)
{
    SdlBuffer instBuffer(mDevice, sizeof(float) * 4 + sizeof(float) * 16);
    
    engine::graphics::Matrix mat;
    mat.translate(dst.x, dst.y);
    mat.scale(dst.width, dst.height);

    instBuffer.writer()
        .append(color)
        .append(mat)
        .release();

    SDL_GPUColorTargetInfo targetInfo{};
    targetInfo.texture = mCurrentSwapchainTexture;
    targetInfo.load_op = SDL_GPU_LOADOP_LOAD;
    targetInfo.store_op = SDL_GPU_STOREOP_STORE;

    SDL_GPURenderPass* pass = SDL_BeginGPURenderPass(mCurrentCmd, &targetInfo, 1, nullptr);
    if (pass) {
        SDL_BindGPUGraphicsPipeline(pass, mColorPipeline);

        SDL_GPUBufferBinding vertexBinding{};
        vertexBinding.buffer = mQuadVertexBuffer->getGpuBuffer();
        vertexBinding.offset = 0;
        SDL_BindGPUVertexBuffers(pass, 0, &vertexBinding, 1);

        SDL_GPUBufferBinding instBinding{};
        instBinding.buffer = instBuffer.getGpuBuffer();
        instBinding.offset = 0;
        SDL_BindGPUVertexBuffers(pass, 1, &instBinding, 1);

        SDL_PushGPUVertexUniformData(mCurrentCmd, 0, mWorldMatrix.constData(), 64);

        SDL_DrawGPUPrimitives(pass, 4, 1, 0, 0);
        SDL_EndGPURenderPass(pass);
    }
}

void SdlRenderer::drawImage(const engine::graphics::Image& img, engine::Types::Rect<> dst, engine::Types::Rect<> src)
{
}

TTF_Font* SdlRenderer::getFont(int32_t size)
{
    auto it = mFonts.find(size);
    if (it != mFonts.end()) {
        return it->second;
    }

    TTF_Font* font = TTF_OpenFont(mFontPath.c_str(), size);
    if (!font) {
        std::cerr << "TTF_OpenFont failed for: " << mFontPath << " size: " << size << " err: " << SDL_GetError() << std::endl;
        return nullptr;
    }
    mFonts[size] = font;
    return font;
}

void SdlRenderer::drawText(const engine::Types::Rect<>& dst, const std::string& text, const engine::graphics::Color& color, int32_t size, engine::Types::TextAlign align, std::string type, bool shadow)
{
    if (text.empty()) return;

    if (shadow) {
        drawText({dst.x + 1, dst.y + 1, dst.width, dst.height}, text, {0.0f, 0.0f, 0.0f, 0.5f}, size, align, type, false);
    }

    TTF_Font* font = getFont(size >= 0 ? size : 24);
    if (!font) return;

    SDL_Color sdlColor{
        static_cast<uint8_t>(color.r() * 255),
        static_cast<uint8_t>(color.g() * 255),
        static_cast<uint8_t>(color.b() * 255),
        static_cast<uint8_t>(color.a() * 255)
    };

    SDL_Surface* surface = TTF_RenderText_Blended(font, text.c_str(), text.length(), sdlColor);
    if (!surface) return;

    SDL_Surface* converted = SDL_ConvertSurface(surface, SDL_PIXELFORMAT_RGBA32);
    SDL_DestroySurface(surface);
    if (!converted) return;

    auto tex = std::make_unique<SdlTexture>(mDevice, converted->w, converted->h, 1);
    uint32_t sizeBytes = converted->w * converted->h * 4;
    SDL_GPUTransferBufferCreateInfo transferInfo{};
    transferInfo.usage = SDL_GPU_TRANSFERBUFFERUSAGE_UPLOAD;
    transferInfo.size = sizeBytes;
    SDL_GPUTransferBuffer* transferBuffer = SDL_CreateGPUTransferBuffer(mDevice, &transferInfo);
    if (transferBuffer) {
        void* ptr = SDL_MapGPUTransferBuffer(mDevice, transferBuffer, false);
        if (ptr) {
            std::memcpy(ptr, converted->pixels, sizeBytes);
            SDL_UnmapGPUTransferBuffer(mDevice, transferBuffer);
        }

        SDL_GPUCommandBuffer* cmd = SDL_AcquireGPUCommandBuffer(mDevice);
        if (cmd) {
            SDL_GPUCopyPass* copyPass = SDL_BeginGPUCopyPass(cmd);
            if (copyPass) {
                SDL_GPUTextureTransferInfo sourceInfo{};
                sourceInfo.transfer_buffer = transferBuffer;
                sourceInfo.offset = 0;

                SDL_GPUTextureRegion destRegion{};
                destRegion.texture = tex->getGpuTexture();
                destRegion.mip_level = 0;
                destRegion.layer = 0;
                destRegion.w = converted->w;
                destRegion.h = converted->h;
                destRegion.d = 1;

                SDL_UploadToGPUTexture(copyPass, &sourceInfo, &destRegion, false);
                SDL_EndGPUCopyPass(copyPass);
            }
            SDL_SubmitGPUCommandBuffer(cmd);
        }
        SDL_ReleaseGPUTransferBuffer(mDevice, transferBuffer);
    }
    SDL_DestroySurface(converted);

    int32_t x = dst.x;
    int32_t y = dst.y;

    if (align.is(engine::Types::TextAlign::Right)) {
        x = dst.x + dst.width - tex->width();
    } else if (align.is(engine::Types::TextAlign::CentreH)) {
        x = dst.x + (dst.width - (int32_t)tex->width()) / 2;
    }

    if (align.is(engine::Types::TextAlign::Bottom)) {
        y = dst.y + dst.height - tex->height();
    } else if (align.is(engine::Types::TextAlign::CentreV)) {
        y = dst.y + (dst.height - (int32_t)tex->height()) / 2;
    }

    SdlBuffer instBuffer(mDevice, sizeof(float) * 4);
    instBuffer.writer()
        .append(Vector4D(x, y, tex->width(), tex->height()))
        .release();

    SDL_GPUColorTargetInfo targetInfo{};
    targetInfo.texture = mCurrentSwapchainTexture;
    targetInfo.load_op = SDL_GPU_LOADOP_LOAD;
    targetInfo.store_op = SDL_GPU_STOREOP_STORE;

    SDL_GPURenderPass* pass = SDL_BeginGPURenderPass(mCurrentCmd, &targetInfo, 1, nullptr);
    if (pass) {
        SDL_BindGPUGraphicsPipeline(pass, mFBO2DPipeline); 

        SDL_GPUBufferBinding vertexBinding{};
        vertexBinding.buffer = mQuadVertexBuffer->getGpuBuffer();
        vertexBinding.offset = 0;
        SDL_BindGPUVertexBuffers(pass, 0, &vertexBinding, 1);

        SDL_GPUBufferBinding instBinding{};
        instBinding.buffer = instBuffer.getGpuBuffer();
        instBinding.offset = 0;
        SDL_BindGPUVertexBuffers(pass, 1, &instBinding, 1);

        SDL_GPUTextureSamplerBinding samplerBinding{};
        samplerBinding.texture = tex->getGpuTexture();
        samplerBinding.sampler = mSampler;
        SDL_BindGPUFragmentSamplers(pass, 0, &samplerBinding, 1);

        SDL_PushGPUVertexUniformData(mCurrentCmd, 0, mWorldMatrix.constData(), 64);

        SDL_DrawGPUPrimitives(pass, 4, 1, 0, 0);
        SDL_EndGPURenderPass(pass);
    }
}

void SdlRenderer::drawOverlay(const engine::Types::Point<>& dst, engine::Overlay& overlay, uint32_t lightsCount, float mod)
{
    mWorldMatrix.translate(dst.x, dst.y);

    const auto& bg = overlay.background();
    SDL_GPUColorTargetInfo fboTarget{};
    fboTarget.texture = mFBO;
    fboTarget.clear_color = SDL_FColor{bg.r(), bg.g(), bg.b(), 1.0f};
    fboTarget.load_op = SDL_GPU_LOADOP_CLEAR;
    fboTarget.store_op = SDL_GPU_STOREOP_STORE;

    SDL_GPURenderPass* pass = SDL_BeginGPURenderPass(mCurrentCmd, &fboTarget, 1, nullptr);
    if (pass) {
        SDL_BindGPUGraphicsPipeline(pass, mPointLightPipeline);

        SDL_GPUBufferBinding vertexBinding{};
        vertexBinding.buffer = mQuadVertexBuffer->getGpuBuffer();
        vertexBinding.offset = 0;
        SDL_BindGPUVertexBuffers(pass, 0, &vertexBinding, 1);

        SDL_GPUBufferBinding instBinding{};
        instBinding.buffer = static_cast<SdlBuffer&>(overlay.lightsBuffer()).getGpuBuffer();
        instBinding.offset = 0;
        SDL_BindGPUVertexBuffers(pass, 1, &instBinding, 1);

        SDL_PushGPUVertexUniformData(mCurrentCmd, 0, mWorldMatrix.constData(), 64);
        
        float iModVal = std::abs(mod - 1.0f);
        SDL_PushGPUFragmentUniformData(mCurrentCmd, 0, &iModVal, sizeof(float));

        SDL_DrawGPUPrimitives(pass, 4, lightsCount, 0, 0);
        SDL_EndGPURenderPass(pass);
    }

    mWorldMatrix.translate(-dst.x, -dst.y);

    SDL_GPUColorTargetInfo screenTarget{};
    screenTarget.texture = mCurrentSwapchainTexture;
    screenTarget.load_op = SDL_GPU_LOADOP_LOAD;
    screenTarget.store_op = SDL_GPU_STOREOP_STORE;

    SDL_GPURenderPass* compositePass = SDL_BeginGPURenderPass(mCurrentCmd, &screenTarget, 1, nullptr);
    if (compositePass) {
        SDL_BindGPUGraphicsPipeline(compositePass, mFBO2DPipeline);

        SDL_GPUBufferBinding vertexBinding{};
        vertexBinding.buffer = mQuadVertexBuffer->getGpuBuffer();
        vertexBinding.offset = 0;
        SDL_BindGPUVertexBuffers(compositePass, 0, &vertexBinding, 1);

        SDL_GPUBufferBinding instBinding{};
        instBinding.buffer = mOverlayBuffer->getGpuBuffer();
        instBinding.offset = 0;
        SDL_BindGPUVertexBuffers(compositePass, 1, &instBinding, 1);

        SDL_GPUTextureSamplerBinding samplerBinding{};
        samplerBinding.texture = mFBO;
        samplerBinding.sampler = mSampler;
        SDL_BindGPUFragmentSamplers(compositePass, 0, &samplerBinding, 1);

        SDL_PushGPUVertexUniformData(mCurrentCmd, 0, mWorldMatrix.constData(), 64);

        SDL_DrawGPUPrimitives(compositePass, 4, 1, 0, 0);
        SDL_EndGPURenderPass(compositePass);
    }
}

void SdlRenderer::drawParticles(const engine::Types::Point<>& dst, const engine::Particles& particles, bool round)
{
    mWorldMatrix.translate(dst.x, dst.y);

    SDL_GPUColorTargetInfo targetInfo{};
    targetInfo.texture = mCurrentSwapchainTexture;
    targetInfo.load_op = SDL_GPU_LOADOP_LOAD;
    targetInfo.store_op = SDL_GPU_STOREOP_STORE;

    SDL_GPURenderPass* pass = SDL_BeginGPURenderPass(mCurrentCmd, &targetInfo, 1, nullptr);
    if (pass) {
        SDL_BindGPUGraphicsPipeline(pass, mParticle2DPipeline);

        SDL_GPUBufferBinding vertexBinding{};
        vertexBinding.buffer = mQuadVertexBuffer->getGpuBuffer();
        vertexBinding.offset = 0;
        SDL_BindGPUVertexBuffers(pass, 0, &vertexBinding, 1);

        SDL_GPUBufferBinding instBinding{};
        instBinding.buffer = static_cast<const SdlBuffer&>(particles.buffer()).getGpuBuffer();
        instBinding.offset = 0;
        SDL_BindGPUVertexBuffers(pass, 1, &instBinding, 1);

        SDL_PushGPUVertexUniformData(mCurrentCmd, 0, mWorldMatrix.constData(), 64);

        int uRoundVal = round ? 1 : 0;
        SDL_PushGPUFragmentUniformData(mCurrentCmd, 0, &uRoundVal, sizeof(int));

        SDL_DrawGPUPrimitives(pass, 4, particles.size(), 0, 0);
        SDL_EndGPURenderPass(pass);
    }

    mWorldMatrix.translate(-dst.x, -dst.y);
}

void SdlRenderer::drawTextures(const engine::Types::Point<>& dst, engine::graphics::Texture* texture, engine::graphics::Buffer* buffer, uint32_t count)
{
    if (count == 0) return;
    mWorldMatrix.translate(dst.x, dst.y);

    SDL_GPUColorTargetInfo targetInfo{};
    targetInfo.texture = mCurrentSwapchainTexture;
    targetInfo.load_op = SDL_GPU_LOADOP_LOAD;
    targetInfo.store_op = SDL_GPU_STOREOP_STORE;

    SDL_GPUDepthStencilTargetInfo depthTarget{};
    depthTarget.texture = mDepthTexture;
    depthTarget.load_op = SDL_GPU_LOADOP_LOAD;
    depthTarget.store_op = SDL_GPU_STOREOP_STORE;

    SDL_GPURenderPass* pass = SDL_BeginGPURenderPass(mCurrentCmd, &targetInfo, 1, &depthTarget);
    if (pass) {
        SDL_BindGPUGraphicsPipeline(pass, mTexture2DPipeline);

        SDL_GPUBufferBinding vertexBinding{};
        vertexBinding.buffer = mQuadVertexBuffer->getGpuBuffer();
        vertexBinding.offset = 0;
        SDL_BindGPUVertexBuffers(pass, 0, &vertexBinding, 1);

        SDL_GPUBufferBinding instBinding{};
        instBinding.buffer = static_cast<SdlBuffer*>(buffer)->getGpuBuffer();
        instBinding.offset = 0;
        SDL_BindGPUVertexBuffers(pass, 1, &instBinding, 1);

        SDL_GPUTextureSamplerBinding samplerBinding{};
        samplerBinding.texture = static_cast<SdlTexture*>(texture)->getGpuTexture();
        samplerBinding.sampler = mSampler;
        SDL_BindGPUFragmentSamplers(pass, 0, &samplerBinding, 1);

        struct { float mat[16]; int reverseY; } uniforms;
        std::memcpy(uniforms.mat, mWorldMatrix.constData(), 64);
        uniforms.reverseY = 0;
        SDL_PushGPUVertexUniformData(mCurrentCmd, 0, &uniforms, sizeof(uniforms));

        SDL_DrawGPUPrimitives(pass, 4, count, 0, 0);
        SDL_EndGPURenderPass(pass);
    }

    mWorldMatrix.translate(-dst.x, -dst.y);
}

void SdlRenderer::drawTexturesAnim(const engine::Types::Point<>& dst, engine::graphics::Texture* texture, engine::graphics::Buffer* buffer, float_t animFrame, uint32_t count)
{
    if (count == 0) return;

    mWorldMatrix.translate(dst.x, dst.y);

    SDL_GPUColorTargetInfo targetInfo{};
    targetInfo.texture = mCurrentSwapchainTexture;
    targetInfo.load_op = SDL_GPU_LOADOP_LOAD;
    targetInfo.store_op = SDL_GPU_STOREOP_STORE;

    SDL_GPUDepthStencilTargetInfo depthTarget{};
    depthTarget.texture = mDepthTexture;
    depthTarget.load_op = SDL_GPU_LOADOP_LOAD;
    depthTarget.store_op = SDL_GPU_STOREOP_STORE;

    SDL_GPURenderPass* pass = SDL_BeginGPURenderPass(mCurrentCmd, &targetInfo, 1, &depthTarget);
    if (pass) {
        SDL_BindGPUGraphicsPipeline(pass, mTextureAnim2DPipeline);

        SDL_GPUBufferBinding vertexBinding{};
        vertexBinding.buffer = mQuadVertexBuffer->getGpuBuffer();
        vertexBinding.offset = 0;
        SDL_BindGPUVertexBuffers(pass, 0, &vertexBinding, 1);

        SDL_GPUBufferBinding instBinding{};
        instBinding.buffer = static_cast<SdlBuffer*>(buffer)->getGpuBuffer();
        instBinding.offset = 0;
        SDL_BindGPUVertexBuffers(pass, 1, &instBinding, 1);

        SDL_GPUTextureSamplerBinding samplerBinding{};
        samplerBinding.texture = static_cast<SdlTexture*>(texture)->getGpuTexture();
        samplerBinding.sampler = mSampler;
        SDL_BindGPUFragmentSamplers(pass, 0, &samplerBinding, 1);

        struct { float mat[16]; float animFrame; } uniforms;
        std::memcpy(uniforms.mat, mWorldMatrix.constData(), 64);
        uniforms.animFrame = animFrame;
        SDL_PushGPUVertexUniformData(mCurrentCmd, 0, &uniforms, sizeof(uniforms));

        SDL_DrawGPUPrimitives(pass, 4, count, 0, 0);
        SDL_EndGPURenderPass(pass);
    }

    mWorldMatrix.translate(-dst.x, -dst.y);
}

void SdlRenderer::rotate(float_t deg)
{
    mWorldMatrix.rotate(deg);
}

void SdlRenderer::translate(int32_t x, int32_t y)
{
    mWorldMatrix.translate(x, y);
}

void SdlRenderer::save()
{
    mMatrixStack.push_back(mWorldMatrix);
}

void SdlRenderer::restore()
{
    if (!mMatrixStack.empty()) {
        mWorldMatrix = mMatrixStack.back();
        mMatrixStack.pop_back();
    }
}

std::unique_ptr<engine::graphics::Buffer> SdlRenderer::createBuffer(uint32_t size) const
{
    return std::make_unique<SdlBuffer>(mDevice, size);
}

std::unique_ptr<engine::graphics::Texture> SdlRenderer::createTexture(uint32_t w, uint32_t h, uint32_t layers) const
{
    return std::make_unique<SdlTexture>(mDevice, w, h, layers);
}

SDL_GPUShader* SdlRenderer::loadShader(const std::string& filename, SDL_GPUShaderStage stage, uint32_t samplerCount, uint32_t uniformBufferCount)
{
    std::ifstream file(filename, std::ios::binary | std::ios::ate);
    if (!file.is_open()) {
        std::cerr << "Failed to open shader: " << filename << std::endl;
        return nullptr;
    }
    size_t size = file.tellg();
    std::vector<char> buffer(size);
    file.seekg(0);
    file.read(buffer.data(), size);

    SDL_GPUShaderCreateInfo createInfo{};
    createInfo.code = reinterpret_cast<const uint8_t*>(buffer.data());
    createInfo.code_size = size;
    createInfo.entrypoint = "main";
    createInfo.stage = stage;
    createInfo.format = SDL_GPU_SHADERFORMAT_SPIRV;
    createInfo.num_samplers = samplerCount;
    createInfo.num_uniform_buffers = uniformBufferCount;

    return SDL_CreateGPUShader(mDevice, &createInfo);
}

void SdlRenderer::initContext()
{
    mFontPath = context().assetManager().fontPath("FantasqueSansMono-Regular.otf");

    // Load shaders
    mTexture2DVert = loadShader("assets/shader/texture2d.vert.spv", SDL_GPU_SHADERSTAGE_VERTEX, 0, 1);
    mTexture2DFrag = loadShader("assets/shader/texture2d.frag.spv", SDL_GPU_SHADERSTAGE_FRAGMENT, 1, 0);
    mTextureAnim2DVert = loadShader("assets/shader/textureanim2d.vert.spv", SDL_GPU_SHADERSTAGE_VERTEX, 0, 1);
    mTextureAnim2DFrag = loadShader("assets/shader/textureanim2d.frag.spv", SDL_GPU_SHADERSTAGE_FRAGMENT, 1, 0);
    mPointLightVert = loadShader("assets/shader/pointlight.vert.spv", SDL_GPU_SHADERSTAGE_VERTEX, 0, 1);
    mPointLightFrag = loadShader("assets/shader/pointlight.frag.spv", SDL_GPU_SHADERSTAGE_FRAGMENT, 0, 1);
    mColorVert = loadShader("assets/shader/color2d.vert.spv", SDL_GPU_SHADERSTAGE_VERTEX, 0, 1);
    mColorFrag = loadShader("assets/shader/color2d.frag.spv", SDL_GPU_SHADERSTAGE_FRAGMENT, 0, 0);
    mParticle2DVert = loadShader("assets/shader/particle2d.vert.spv", SDL_GPU_SHADERSTAGE_VERTEX, 0, 1);
    mParticle2DFrag = loadShader("assets/shader/particle2d.frag.spv", SDL_GPU_SHADERSTAGE_FRAGMENT, 0, 1);
    mFBO2DVert = loadShader("assets/shader/fbo2d.vert.spv", SDL_GPU_SHADERSTAGE_VERTEX, 0, 1);
    mFBO2DFrag = loadShader("assets/shader/fbo2d.frag.spv", SDL_GPU_SHADERSTAGE_FRAGMENT, 1, 0);

    // Create Sampler
    SDL_GPUSamplerCreateInfo samplerInfo{};
    samplerInfo.min_filter = SDL_GPU_FILTER_LINEAR;
    samplerInfo.mag_filter = SDL_GPU_FILTER_LINEAR;
    samplerInfo.mipmap_mode = SDL_GPU_SAMPLERMIPMAPMODE_LINEAR;
    samplerInfo.address_mode_u = SDL_GPU_SAMPLERADDRESSMODE_CLAMP_TO_EDGE;
    samplerInfo.address_mode_v = SDL_GPU_SAMPLERADDRESSMODE_CLAMP_TO_EDGE;
    mSampler = SDL_CreateGPUSampler(mDevice, &samplerInfo);

    // Pipeline target format and default blend state
    SDL_GPUTextureFormat targetFormat = SDL_GetGPUSwapchainTextureFormat(mDevice, mWindow);

    SDL_GPUColorTargetDescription defaultColorTarget{};
    defaultColorTarget.format = targetFormat;
    defaultColorTarget.blend_state.enable_blend = true;
    defaultColorTarget.blend_state.src_color_blendfactor = SDL_GPU_BLENDFACTOR_SRC_ALPHA;
    defaultColorTarget.blend_state.dst_color_blendfactor = SDL_GPU_BLENDFACTOR_ONE_MINUS_SRC_ALPHA;
    defaultColorTarget.blend_state.color_blend_op = SDL_GPU_BLENDOP_ADD;
    defaultColorTarget.blend_state.src_alpha_blendfactor = SDL_GPU_BLENDFACTOR_SRC_ALPHA;
    defaultColorTarget.blend_state.dst_alpha_blendfactor = SDL_GPU_BLENDFACTOR_ONE_MINUS_SRC_ALPHA;
    defaultColorTarget.blend_state.alpha_blend_op = SDL_GPU_BLENDOP_ADD;
    defaultColorTarget.blend_state.enable_color_write_mask = true;
    defaultColorTarget.blend_state.color_write_mask = SDL_GPU_COLORCOMPONENT_R | SDL_GPU_COLORCOMPONENT_G | SDL_GPU_COLORCOMPONENT_B | SDL_GPU_COLORCOMPONENT_A;

    // 1. Texture2D Pipeline
    {
        SDL_GPUGraphicsPipelineCreateInfo pipelineInfo{};
        pipelineInfo.vertex_shader = mTexture2DVert;
        pipelineInfo.fragment_shader = mTexture2DFrag;
        pipelineInfo.primitive_type = SDL_GPU_PRIMITIVETYPE_TRIANGLESTRIP;

        SDL_GPUVertexBufferDescription bufferDescs[2]{};
        bufferDescs[0].slot = 0;
        bufferDescs[0].pitch = sizeof(float) * 2;
        bufferDescs[0].input_rate = SDL_GPU_VERTEXINPUTRATE_VERTEX;
        
        bufferDescs[1].slot = 1;
        bufferDescs[1].pitch = sizeof(float) * 9;
        bufferDescs[1].input_rate = SDL_GPU_VERTEXINPUTRATE_INSTANCE;
        bufferDescs[1].instance_step_rate = 0;

        SDL_GPUVertexAttribute attributeDescs[4]{};
        attributeDescs[0].location = 0;
        attributeDescs[0].buffer_slot = 0;
        attributeDescs[0].format = SDL_GPU_VERTEXELEMENTFORMAT_FLOAT2;
        attributeDescs[0].offset = 0;

        attributeDescs[1].location = 1;
        attributeDescs[1].buffer_slot = 1;
        attributeDescs[1].format = SDL_GPU_VERTEXELEMENTFORMAT_FLOAT4;
        attributeDescs[1].offset = 0;

        attributeDescs[2].location = 2;
        attributeDescs[2].buffer_slot = 1;
        attributeDescs[2].format = SDL_GPU_VERTEXELEMENTFORMAT_FLOAT4;
        attributeDescs[2].offset = 16;

        attributeDescs[3].location = 3;
        attributeDescs[3].buffer_slot = 1;
        attributeDescs[3].format = SDL_GPU_VERTEXELEMENTFORMAT_FLOAT;
        attributeDescs[3].offset = 32;

        pipelineInfo.vertex_input_state.num_vertex_buffers = 2;
        pipelineInfo.vertex_input_state.vertex_buffer_descriptions = bufferDescs;
        pipelineInfo.vertex_input_state.num_vertex_attributes = 4;
        pipelineInfo.vertex_input_state.vertex_attributes = attributeDescs;

        pipelineInfo.target_info.num_color_targets = 1;
        pipelineInfo.target_info.color_target_descriptions = &defaultColorTarget;
        pipelineInfo.target_info.has_depth_stencil_target = true;
        pipelineInfo.target_info.depth_stencil_format = SDL_GPU_TEXTUREFORMAT_D16_UNORM;

        pipelineInfo.depth_stencil_state.enable_depth_test = true;
        pipelineInfo.depth_stencil_state.enable_depth_write = true;
        pipelineInfo.depth_stencil_state.compare_op = SDL_GPU_COMPAREOP_LESS_OR_EQUAL;

        mTexture2DPipeline = SDL_CreateGPUGraphicsPipeline(mDevice, &pipelineInfo);
    }

    // 2. TextureAnim2D Pipeline
    {
        SDL_GPUGraphicsPipelineCreateInfo pipelineInfo{};
        pipelineInfo.vertex_shader = mTextureAnim2DVert;
        pipelineInfo.fragment_shader = mTextureAnim2DFrag;
        pipelineInfo.primitive_type = SDL_GPU_PRIMITIVETYPE_TRIANGLESTRIP;

        SDL_GPUVertexBufferDescription bufferDescs[2]{};
        bufferDescs[0].slot = 0;
        bufferDescs[0].pitch = sizeof(float) * 2;
        bufferDescs[0].input_rate = SDL_GPU_VERTEXINPUTRATE_VERTEX;

        bufferDescs[1].slot = 1;
        bufferDescs[1].pitch = sizeof(float) * 12; // 4 + 1 + 4 + 2 + 1 = 12
        bufferDescs[1].input_rate = SDL_GPU_VERTEXINPUTRATE_INSTANCE;
        bufferDescs[1].instance_step_rate = 0;

        SDL_GPUVertexAttribute attributeDescs[6]{};
        attributeDescs[0].location = 0;
        attributeDescs[0].buffer_slot = 0;
        attributeDescs[0].format = SDL_GPU_VERTEXELEMENTFORMAT_FLOAT2;
        attributeDescs[0].offset = 0;

        attributeDescs[1].location = 1;
        attributeDescs[1].buffer_slot = 1;
        attributeDescs[1].format = SDL_GPU_VERTEXELEMENTFORMAT_FLOAT4;
        attributeDescs[1].offset = 0;

        attributeDescs[2].location = 2;
        attributeDescs[2].buffer_slot = 1;
        attributeDescs[2].format = SDL_GPU_VERTEXELEMENTFORMAT_FLOAT;
        attributeDescs[2].offset = 16;

        attributeDescs[3].location = 3;
        attributeDescs[3].buffer_slot = 1;
        attributeDescs[3].format = SDL_GPU_VERTEXELEMENTFORMAT_FLOAT4;
        attributeDescs[3].offset = 20;

        attributeDescs[4].location = 4;
        attributeDescs[4].buffer_slot = 1;
        attributeDescs[4].format = SDL_GPU_VERTEXELEMENTFORMAT_FLOAT2;
        attributeDescs[4].offset = 36;

        attributeDescs[5].location = 5;
        attributeDescs[5].buffer_slot = 1;
        attributeDescs[5].format = SDL_GPU_VERTEXELEMENTFORMAT_FLOAT;
        attributeDescs[5].offset = 44;

        pipelineInfo.vertex_input_state.num_vertex_buffers = 2;
        pipelineInfo.vertex_input_state.vertex_buffer_descriptions = bufferDescs;
        pipelineInfo.vertex_input_state.num_vertex_attributes = 6;
        pipelineInfo.vertex_input_state.vertex_attributes = attributeDescs;

        pipelineInfo.target_info.num_color_targets = 1;
        pipelineInfo.target_info.color_target_descriptions = &defaultColorTarget;
        pipelineInfo.target_info.has_depth_stencil_target = true;
        pipelineInfo.target_info.depth_stencil_format = SDL_GPU_TEXTUREFORMAT_D16_UNORM;

        pipelineInfo.depth_stencil_state.enable_depth_test = true;
        pipelineInfo.depth_stencil_state.enable_depth_write = true;
        pipelineInfo.depth_stencil_state.compare_op = SDL_GPU_COMPAREOP_LESS_OR_EQUAL;

        mTextureAnim2DPipeline = SDL_CreateGPUGraphicsPipeline(mDevice, &pipelineInfo);
    }

    // 3. PointLight Pipeline (renders to FBO, so target is RGBA_UNORM and uses MAX blend)
    {
        SDL_GPUGraphicsPipelineCreateInfo pipelineInfo{};
        pipelineInfo.vertex_shader = mPointLightVert;
        pipelineInfo.fragment_shader = mPointLightFrag;
        pipelineInfo.primitive_type = SDL_GPU_PRIMITIVETYPE_TRIANGLESTRIP;

        SDL_GPUVertexBufferDescription bufferDescs[2]{};
        bufferDescs[0].slot = 0;
        bufferDescs[0].pitch = sizeof(float) * 2;
        bufferDescs[0].input_rate = SDL_GPU_VERTEXINPUTRATE_VERTEX;

        bufferDescs[1].slot = 1;
        bufferDescs[1].pitch = sizeof(float) * 12;
        bufferDescs[1].input_rate = SDL_GPU_VERTEXINPUTRATE_INSTANCE;
        bufferDescs[1].instance_step_rate = 0;

        SDL_GPUVertexAttribute attributeDescs[4]{};
        attributeDescs[0].location = 0;
        attributeDescs[0].buffer_slot = 0;
        attributeDescs[0].format = SDL_GPU_VERTEXELEMENTFORMAT_FLOAT2;
        attributeDescs[0].offset = 0;

        attributeDescs[1].location = 1;
        attributeDescs[1].buffer_slot = 1;
        attributeDescs[1].format = SDL_GPU_VERTEXELEMENTFORMAT_FLOAT4;
        attributeDescs[1].offset = 0;

        attributeDescs[2].location = 2;
        attributeDescs[2].buffer_slot = 1;
        attributeDescs[2].format = SDL_GPU_VERTEXELEMENTFORMAT_FLOAT4;
        attributeDescs[2].offset = 16;

        attributeDescs[3].location = 3;
        attributeDescs[3].buffer_slot = 1;
        attributeDescs[3].format = SDL_GPU_VERTEXELEMENTFORMAT_FLOAT4;
        attributeDescs[3].offset = 32;

        pipelineInfo.vertex_input_state.num_vertex_buffers = 2;
        pipelineInfo.vertex_input_state.vertex_buffer_descriptions = bufferDescs;
        pipelineInfo.vertex_input_state.num_vertex_attributes = 4;
        pipelineInfo.vertex_input_state.vertex_attributes = attributeDescs;

        SDL_GPUColorTargetDescription fboTarget{};
        fboTarget.format = SDL_GPU_TEXTUREFORMAT_R8G8B8A8_UNORM;
        fboTarget.blend_state.enable_blend = true;
        fboTarget.blend_state.src_color_blendfactor = SDL_GPU_BLENDFACTOR_SRC_ALPHA;
        fboTarget.blend_state.dst_color_blendfactor = SDL_GPU_BLENDFACTOR_ONE_MINUS_SRC_ALPHA;
        fboTarget.blend_state.color_blend_op = SDL_GPU_BLENDOP_ADD;
        fboTarget.blend_state.src_alpha_blendfactor = SDL_GPU_BLENDFACTOR_ONE;
        fboTarget.blend_state.dst_alpha_blendfactor = SDL_GPU_BLENDFACTOR_ONE;
        fboTarget.blend_state.alpha_blend_op = SDL_GPU_BLENDOP_MAX;
        fboTarget.blend_state.enable_color_write_mask = true;
        fboTarget.blend_state.color_write_mask = SDL_GPU_COLORCOMPONENT_R | SDL_GPU_COLORCOMPONENT_G | SDL_GPU_COLORCOMPONENT_B | SDL_GPU_COLORCOMPONENT_A;

        pipelineInfo.target_info.num_color_targets = 1;
        pipelineInfo.target_info.color_target_descriptions = &fboTarget;

        mPointLightPipeline = SDL_CreateGPUGraphicsPipeline(mDevice, &pipelineInfo);
    }

    // 4. Color Pipeline
    {
        SDL_GPUGraphicsPipelineCreateInfo pipelineInfo{};
        pipelineInfo.vertex_shader = mColorVert;
        pipelineInfo.fragment_shader = mColorFrag;
        pipelineInfo.primitive_type = SDL_GPU_PRIMITIVETYPE_TRIANGLESTRIP;

        SDL_GPUVertexBufferDescription bufferDescs[2]{};
        bufferDescs[0].slot = 0;
        bufferDescs[0].pitch = sizeof(float) * 2;
        bufferDescs[0].input_rate = SDL_GPU_VERTEXINPUTRATE_VERTEX;

        bufferDescs[1].slot = 1;
        bufferDescs[1].pitch = sizeof(float) * 20; // 4 + 16 = 20
        bufferDescs[1].input_rate = SDL_GPU_VERTEXINPUTRATE_INSTANCE;
        bufferDescs[1].instance_step_rate = 0;

        SDL_GPUVertexAttribute attributeDescs[6]{};
        attributeDescs[0].location = 0;
        attributeDescs[0].buffer_slot = 0;
        attributeDescs[0].format = SDL_GPU_VERTEXELEMENTFORMAT_FLOAT2;
        attributeDescs[0].offset = 0;

        attributeDescs[1].location = 1;
        attributeDescs[1].buffer_slot = 1;
        attributeDescs[1].format = SDL_GPU_VERTEXELEMENTFORMAT_FLOAT4;
        attributeDescs[1].offset = 0;

        // matrix col 0, 1, 2, 3
        attributeDescs[2].location = 2;
        attributeDescs[2].buffer_slot = 1;
        attributeDescs[2].format = SDL_GPU_VERTEXELEMENTFORMAT_FLOAT4;
        attributeDescs[2].offset = 16;

        attributeDescs[3].location = 3;
        attributeDescs[3].buffer_slot = 1;
        attributeDescs[3].format = SDL_GPU_VERTEXELEMENTFORMAT_FLOAT4;
        attributeDescs[3].offset = 32;

        attributeDescs[4].location = 4;
        attributeDescs[4].buffer_slot = 1;
        attributeDescs[4].format = SDL_GPU_VERTEXELEMENTFORMAT_FLOAT4;
        attributeDescs[4].offset = 48;

        attributeDescs[5].location = 5;
        attributeDescs[5].buffer_slot = 1;
        attributeDescs[5].format = SDL_GPU_VERTEXELEMENTFORMAT_FLOAT4;
        attributeDescs[5].offset = 64;

        pipelineInfo.vertex_input_state.num_vertex_buffers = 2;
        pipelineInfo.vertex_input_state.vertex_buffer_descriptions = bufferDescs;
        pipelineInfo.vertex_input_state.num_vertex_attributes = 6;
        pipelineInfo.vertex_input_state.vertex_attributes = attributeDescs;

        pipelineInfo.target_info.num_color_targets = 1;
        pipelineInfo.target_info.color_target_descriptions = &defaultColorTarget;

        mColorPipeline = SDL_CreateGPUGraphicsPipeline(mDevice, &pipelineInfo);
    }

    // 5. Particle Pipeline
    {
        SDL_GPUGraphicsPipelineCreateInfo pipelineInfo{};
        pipelineInfo.vertex_shader = mParticle2DVert;
        pipelineInfo.fragment_shader = mParticle2DFrag;
        pipelineInfo.primitive_type = SDL_GPU_PRIMITIVETYPE_TRIANGLESTRIP;

        SDL_GPUVertexBufferDescription bufferDescs[2]{};
        bufferDescs[0].slot = 0;
        bufferDescs[0].pitch = sizeof(float) * 2;
        bufferDescs[0].input_rate = SDL_GPU_VERTEXINPUTRATE_VERTEX;

        bufferDescs[1].slot = 1;
        bufferDescs[1].pitch = sizeof(float) * 11; // 4 + 1 + 2 + 2 + 2 = 11
        bufferDescs[1].input_rate = SDL_GPU_VERTEXINPUTRATE_INSTANCE;
        bufferDescs[1].instance_step_rate = 0;

        SDL_GPUVertexAttribute attributeDescs[6]{};
        attributeDescs[0].location = 0;
        attributeDescs[0].buffer_slot = 0;
        attributeDescs[0].format = SDL_GPU_VERTEXELEMENTFORMAT_FLOAT2;
        attributeDescs[0].offset = 0;

        attributeDescs[1].location = 1;
        attributeDescs[1].buffer_slot = 1;
        attributeDescs[1].format = SDL_GPU_VERTEXELEMENTFORMAT_FLOAT4;
        attributeDescs[1].offset = 0;

        attributeDescs[2].location = 2;
        attributeDescs[2].buffer_slot = 1;
        attributeDescs[2].format = SDL_GPU_VERTEXELEMENTFORMAT_FLOAT;
        attributeDescs[2].offset = 16;

        attributeDescs[3].location = 3;
        attributeDescs[3].buffer_slot = 1;
        attributeDescs[3].format = SDL_GPU_VERTEXELEMENTFORMAT_FLOAT2;
        attributeDescs[3].offset = 20;

        attributeDescs[4].location = 4;
        attributeDescs[4].buffer_slot = 1;
        attributeDescs[4].format = SDL_GPU_VERTEXELEMENTFORMAT_FLOAT2;
        attributeDescs[4].offset = 28;

        attributeDescs[5].location = 5;
        attributeDescs[5].buffer_slot = 1;
        attributeDescs[5].format = SDL_GPU_VERTEXELEMENTFORMAT_FLOAT2;
        attributeDescs[5].offset = 36;

        pipelineInfo.vertex_input_state.num_vertex_buffers = 2;
        pipelineInfo.vertex_input_state.vertex_buffer_descriptions = bufferDescs;
        pipelineInfo.vertex_input_state.num_vertex_attributes = 6;
        pipelineInfo.vertex_input_state.vertex_attributes = attributeDescs;

        pipelineInfo.target_info.num_color_targets = 1;
        pipelineInfo.target_info.color_target_descriptions = &defaultColorTarget;

        mParticle2DPipeline = SDL_CreateGPUGraphicsPipeline(mDevice, &pipelineInfo);
    }

    // 6. FBO multiply composite Pipeline
    {
        SDL_GPUGraphicsPipelineCreateInfo pipelineInfo{};
        pipelineInfo.vertex_shader = mFBO2DVert;
        pipelineInfo.fragment_shader = mFBO2DFrag;
        pipelineInfo.primitive_type = SDL_GPU_PRIMITIVETYPE_TRIANGLESTRIP;

        SDL_GPUVertexBufferDescription bufferDescs[2]{};
        bufferDescs[0].slot = 0;
        bufferDescs[0].pitch = sizeof(float) * 2;
        bufferDescs[0].input_rate = SDL_GPU_VERTEXINPUTRATE_VERTEX;

        bufferDescs[1].slot = 1;
        bufferDescs[1].pitch = sizeof(float) * 4;
        bufferDescs[1].input_rate = SDL_GPU_VERTEXINPUTRATE_INSTANCE;
        bufferDescs[1].instance_step_rate = 0;

        SDL_GPUVertexAttribute attributeDescs[2]{};
        attributeDescs[0].location = 0;
        attributeDescs[0].buffer_slot = 0;
        attributeDescs[0].format = SDL_GPU_VERTEXELEMENTFORMAT_FLOAT2;
        attributeDescs[0].offset = 0;

        attributeDescs[1].location = 1;
        attributeDescs[1].buffer_slot = 1;
        attributeDescs[1].format = SDL_GPU_VERTEXELEMENTFORMAT_FLOAT4;
        attributeDescs[1].offset = 0;

        pipelineInfo.vertex_input_state.num_vertex_buffers = 2;
        pipelineInfo.vertex_input_state.vertex_buffer_descriptions = bufferDescs;
        pipelineInfo.vertex_input_state.num_vertex_attributes = 2;
        pipelineInfo.vertex_input_state.vertex_attributes = attributeDescs;

        SDL_GPUColorTargetDescription multiplyTarget{};
        multiplyTarget.format = targetFormat;
        multiplyTarget.blend_state.enable_blend = true;
        multiplyTarget.blend_state.src_color_blendfactor = SDL_GPU_BLENDFACTOR_DST_COLOR;
        multiplyTarget.blend_state.dst_color_blendfactor = SDL_GPU_BLENDFACTOR_ZERO;
        multiplyTarget.blend_state.color_blend_op = SDL_GPU_BLENDOP_ADD;
        multiplyTarget.blend_state.src_alpha_blendfactor = SDL_GPU_BLENDFACTOR_DST_ALPHA;
        multiplyTarget.blend_state.dst_alpha_blendfactor = SDL_GPU_BLENDFACTOR_ZERO;
        multiplyTarget.blend_state.alpha_blend_op = SDL_GPU_BLENDOP_ADD;
        multiplyTarget.blend_state.enable_color_write_mask = true;
        multiplyTarget.blend_state.color_write_mask = SDL_GPU_COLORCOMPONENT_R | SDL_GPU_COLORCOMPONENT_G | SDL_GPU_COLORCOMPONENT_B | SDL_GPU_COLORCOMPONENT_A;

        pipelineInfo.target_info.num_color_targets = 1;
        pipelineInfo.target_info.color_target_descriptions = &multiplyTarget;

        mFBO2DPipeline = SDL_CreateGPUGraphicsPipeline(mDevice, &pipelineInfo);
    }

    // Diagnostic logging
    fprintf(stderr, "=== initContext diagnostics ===\n");
    fprintf(stderr, "Shaders: tex2d_v=%p tex2d_f=%p anim_v=%p anim_f=%p light_v=%p light_f=%p col_v=%p col_f=%p part_v=%p part_f=%p fbo_v=%p fbo_f=%p\n",
        (void*)mTexture2DVert, (void*)mTexture2DFrag, (void*)mTextureAnim2DVert, (void*)mTextureAnim2DFrag,
        (void*)mPointLightVert, (void*)mPointLightFrag, (void*)mColorVert, (void*)mColorFrag,
        (void*)mParticle2DVert, (void*)mParticle2DFrag, (void*)mFBO2DVert, (void*)mFBO2DFrag);
    fprintf(stderr, "Pipelines: tex2d=%p anim2d=%p light=%p color=%p particle=%p fbo=%p\n",
        (void*)mTexture2DPipeline, (void*)mTextureAnim2DPipeline, (void*)mPointLightPipeline,
        (void*)mColorPipeline, (void*)mParticle2DPipeline, (void*)mFBO2DPipeline);
    fprintf(stderr, "Sampler=%p swapchain_format=%d\n", (void*)mSampler, targetFormat);
}
