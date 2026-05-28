#include <algorithm>
#include <cstring>
#include <ui/sdltexture.h>

SdlTexture::SdlTexture(SDL_GPUDevice* device, uint32_t w, uint32_t h, uint32_t layers)
    : engine::graphics::Texture(w, h, layers)
    , mDevice(device)
    , mGpuTexture(nullptr)
    , mWidth(w)
    , mHeight(h)
{
    SDL_GPUTextureCreateInfo textureInfo{};
    textureInfo.type = SDL_GPU_TEXTURETYPE_2D_ARRAY;
    textureInfo.format = SDL_GPU_TEXTUREFORMAT_R8G8B8A8_UNORM;
    textureInfo.usage = SDL_GPU_TEXTUREUSAGE_SAMPLER | SDL_GPU_TEXTUREUSAGE_COLOR_TARGET;
    textureInfo.width = w;
    textureInfo.height = h;
    textureInfo.layer_count_or_depth = (layers > 0) ? layers : 1;
    textureInfo.num_levels = 1;

    mGpuTexture = SDL_CreateGPUTexture(mDevice, &textureInfo);
}

SdlTexture::~SdlTexture()
{
    if (mGpuTexture) {
        SDL_ReleaseGPUTexture(mDevice, mGpuTexture);
    }
}

void SdlTexture::setData(const engine::graphics::Image& image, uint32_t layer)
{
    if (!mGpuTexture) return;

    uint32_t w = std::min(mWidth, image.width());
    uint32_t h = std::min(mHeight, image.height());
    uint32_t size = w * h * 4;

    SDL_GPUTransferBufferCreateInfo transferInfo{};
    transferInfo.usage = SDL_GPU_TRANSFERBUFFERUSAGE_UPLOAD;
    transferInfo.size = size;
    SDL_GPUTransferBuffer* transferBuffer = SDL_CreateGPUTransferBuffer(mDevice, &transferInfo);
    if (!transferBuffer) {
        return;
    }

    void* ptr = SDL_MapGPUTransferBuffer(mDevice, transferBuffer, false);
    if (ptr) {
        if (image.width() == mWidth) {
            std::memcpy(ptr, image.data(), size);
        } else {
            const uint8_t* src = image.data();
            uint8_t* dst = static_cast<uint8_t*>(ptr);
            for (uint32_t y = 0; y < h; ++y) {
                std::memcpy(dst + y * w * 4, src + y * image.width() * 4, w * 4);
            }
        }
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
            destRegion.texture = mGpuTexture;
            destRegion.mip_level = 0;
            destRegion.layer = layer;
            destRegion.w = w;
            destRegion.h = h;
            destRegion.d = 1;

            SDL_UploadToGPUTexture(copyPass, &sourceInfo, &destRegion, false);
            SDL_EndGPUCopyPass(copyPass);
        }
        SDL_SubmitGPUCommandBuffer(cmd);
    }

    SDL_ReleaseGPUTransferBuffer(mDevice, transferBuffer);

    setDimensions(engine::Types::Dimension<>(w, h), layer);
}

uint32_t SdlTexture::width() const
{
    return mWidth;
}

uint32_t SdlTexture::height() const
{
    return mHeight;
}
