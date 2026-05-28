#include <cstring>
#include <engine/graphics/matrix.h>
#include <engine/graphics/quad.h>
#include <engine/graphics/vector.h>
#include <ui/sdlbuffer.h>

SdlBuffer::SdlBuffer(SDL_GPUDevice* device, uint32_t size)
    : mDevice(device)
    , mGpuBuffer(nullptr)
    , mSize(size)
    , mDirty(false)
{
    if (size > 0) {
        resize(size);
    }
}

SdlBuffer::~SdlBuffer()
{
    if (mGpuBuffer) {
        SDL_ReleaseGPUBuffer(mDevice, mGpuBuffer);
    }
}

void SdlBuffer::resize(uint32_t size)
{
    if (size == 0) {
        return;
    }

    if (mGpuBuffer) {
        SDL_ReleaseGPUBuffer(mDevice, mGpuBuffer);
        mGpuBuffer = nullptr;
    }

    mSize = size;
    mStagingData.resize(size, 0);

    SDL_GPUBufferCreateInfo createInfo{};
    createInfo.usage = SDL_GPU_BUFFERUSAGE_VERTEX | SDL_GPU_BUFFERUSAGE_INDEX;
    createInfo.size = size;

    mGpuBuffer = SDL_CreateGPUBuffer(mDevice, &createInfo);
    mDirty = true;
}

void SdlBuffer::upload()
{
    if (!mDirty || mSize == 0 || !mGpuBuffer) {
        return;
    }

    SDL_GPUTransferBufferCreateInfo transferInfo{};
    transferInfo.usage = SDL_GPU_TRANSFERBUFFERUSAGE_UPLOAD;
    transferInfo.size = mSize;
    SDL_GPUTransferBuffer* transferBuffer = SDL_CreateGPUTransferBuffer(mDevice, &transferInfo);
    if (!transferBuffer) {
        return;
    }

    void* ptr = SDL_MapGPUTransferBuffer(mDevice, transferBuffer, false);
    if (ptr) {
        std::memcpy(ptr, mStagingData.data(), mSize);
        SDL_UnmapGPUTransferBuffer(mDevice, transferBuffer);
    }

    SDL_GPUCommandBuffer* cmd = SDL_AcquireGPUCommandBuffer(mDevice);
    if (cmd) {
        SDL_GPUCopyPass* copyPass = SDL_BeginGPUCopyPass(cmd);
        if (copyPass) {
            SDL_GPUTransferBufferLocation sourceLocation{};
            sourceLocation.transfer_buffer = transferBuffer;
            sourceLocation.offset = 0;

            SDL_GPUBufferRegion destRegion{};
            destRegion.buffer = mGpuBuffer;
            destRegion.offset = 0;
            destRegion.size = mSize;

            SDL_UploadToGPUBuffer(copyPass, &sourceLocation, &destRegion, false);
            SDL_EndGPUCopyPass(copyPass);
        }
        SDL_SubmitGPUCommandBuffer(cmd);
    }

    SDL_ReleaseGPUTransferBuffer(mDevice, transferBuffer);
    mDirty = false;
}

void SdlBuffer::writeStaging(uint32_t offset, const void* data, uint32_t size)
{
    if (offset + size > mStagingData.size()) {
        mStagingData.resize(offset + size);
        mSize = mStagingData.size();
        mDirty = true;
    }
    std::memcpy(mStagingData.data() + offset, data, size);
    mDirty = true;
}

uint32_t SdlBuffer::write(uint32_t offset, const engine::graphics::Color& color)
{
    writeStaging(offset, color.constData(), engine::graphics::Color::Size());
    return engine::graphics::Color::Size();
}

uint32_t SdlBuffer::write(uint32_t offset, const engine::graphics::Matrix& matrix)
{
    writeStaging(offset, matrix.constData(), engine::graphics::Matrix::Size());
    return engine::graphics::Matrix::Size();
}

uint32_t SdlBuffer::write(uint32_t offset, const engine::graphics::Vector2D& vector)
{
    writeStaging(offset, vector.constData(), engine::graphics::Vector2D::Size());
    return engine::graphics::Vector2D::Size();
}

uint32_t SdlBuffer::write(uint32_t offset, const engine::graphics::Vector3D& vector)
{
    writeStaging(offset, vector.constData(), engine::graphics::Vector3D::Size());
    return engine::graphics::Vector3D::Size();
}

uint32_t SdlBuffer::write(uint32_t offset, const engine::graphics::Vector4D& vector)
{
    writeStaging(offset, vector.constData(), engine::graphics::Vector4D::Size());
    return engine::graphics::Vector4D::Size();
}

uint32_t SdlBuffer::write(uint32_t offset, const engine::graphics::Quad<2>& quad)
{
    writeStaging(offset, quad.constData(), engine::graphics::Quad<2>::Size());
    return engine::graphics::Quad<2>::Size();
}

uint32_t SdlBuffer::write(uint32_t offset, int val)
{
    writeStaging(offset, &val, sizeof(int));
    return sizeof(float_t);
}

uint32_t SdlBuffer::write(uint32_t offset, float_t val)
{
    writeStaging(offset, &val, sizeof(float_t));
    return sizeof(float_t);
}

uint32_t SdlBuffer::write(uint32_t offset, uint32_t val)
{
    writeStaging(offset, &val, sizeof(uint32_t));
    return sizeof(uint32_t);
}
