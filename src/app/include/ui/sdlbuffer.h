#pragma once

#include <cmath>
#include <vector>
#include <SDL3/SDL.h>
#include <engine/graphics/buffer.h>
#include <engine/graphics/quad.h>

class SdlBuffer : public engine::graphics::Buffer
{
public:
    SdlBuffer(SDL_GPUDevice* device, uint32_t size = 0);
    virtual ~SdlBuffer();

    virtual void upload() override;
    virtual void resize(uint32_t size) override;

    virtual uint32_t write(uint32_t offset, const engine::graphics::Color& color) override;
    virtual uint32_t write(uint32_t offset, const engine::graphics::Matrix& matrix) override;
    virtual uint32_t write(uint32_t offset, const engine::graphics::Vector2D& vector) override;
    virtual uint32_t write(uint32_t offset, const engine::graphics::Vector3D& vector) override;
    virtual uint32_t write(uint32_t offset, const engine::graphics::Vector4D& vector) override;
    virtual uint32_t write(uint32_t offset, const engine::graphics::Quad<2>& quad) override;

    virtual uint32_t write(uint32_t offset, int val) override;
    virtual uint32_t write(uint32_t offset, float_t val) override;
    virtual uint32_t write(uint32_t offset, uint32_t val) override;

    SDL_GPUBuffer* getGpuBuffer() const { return mGpuBuffer; }
    uint32_t getSize() const { return mSize; }

private:
    void writeStaging(uint32_t offset, const void* data, uint32_t size);

    SDL_GPUDevice* mDevice;
    SDL_GPUBuffer* mGpuBuffer;
    uint32_t mSize;
    std::vector<uint8_t> mStagingData;
    bool mDirty;
};
