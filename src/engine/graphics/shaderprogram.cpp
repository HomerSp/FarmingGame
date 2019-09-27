#include <engine/graphics/buffer.h>
#include <engine/graphics/shaderprogram.h>

using namespace engine;
using namespace engine::graphics;

ShaderProgram::BufferEnabler::BufferEnabler(ShaderProgram& program, engine::graphics::Buffer& buffer, uint32_t startIndex)
    : mProgram(program)
    , mBuffer(&buffer)
    , mStartIndex(startIndex)
{
}

ShaderProgram::BufferEnabler& ShaderProgram::BufferEnabler::append(uint32_t type, uint32_t size, bool divisor, uint32_t repeatCount)
{
    for (uint32_t i = 0; i < repeatCount; i++) {
        mBufferSizes.push_back({type, size, divisor});
    }

    return *this;
}

ShaderProgram::BufferEnabler& ShaderProgram::BufferEnabler::buffer(engine::graphics::Buffer& buffer)
{
    uint32_t oldSize = mBufferSizes.size();
    if (oldSize > 0) {
        release();
    }

    mStartIndex = mStartIndex + oldSize;
    mBuffer = &buffer;
    return *this;
}

void ShaderProgram::BufferEnabler::release()
{
    if (mBufferSizes.empty()) {
        return;
    }

    uint32_t stride = 0;
    for (auto& i: mBufferSizes) {
        stride += i.size;
    }

    mBuffer->bind();

    uint32_t offset = 0;
    for (uint32_t i = 0; i < mBufferSizes.size(); i++) {
        auto b = mBufferSizes[i];
        uint8_t count = b.size / mProgram.typeSize(b.type);
        mProgram.setAttributeBuffer(mStartIndex + i, b.type, offset, count, stride, b.divisor);
        offset += b.size;
    }

    mBuffer->release();
    mBufferSizes.clear();
}

ShaderProgram::BufferEnabler ShaderProgram::bufferEnabler(engine::graphics::Buffer& buffer, uint32_t startIndex)
{
    return ShaderProgram::BufferEnabler(*this, buffer, startIndex);
}