#pragma once

#include <cstdint>
#include <functional>
#include <map>

#include <engine/graphics/vector.h>

namespace engine {
namespace graphics {

class Buffer;
class Matrix;

class ShaderProgram {
private:
    struct BufferSize {
        uint32_t type;
        uint32_t size;
        bool divisor;
    };
public:
    class BufferEnabler {
    public:
        BufferEnabler(ShaderProgram& program, engine::graphics::Buffer& buffer, uint32_t startIndex);

        ShaderProgram::BufferEnabler& append(uint32_t type, uint32_t size, bool divisor = false, uint32_t repeatCount = 1);
        ShaderProgram::BufferEnabler& buffer(engine::graphics::Buffer& buffer);
        void release();

    private:
        ShaderProgram& mProgram;
        engine::graphics::Buffer* mBuffer;
        uint32_t mStartIndex;
        std::vector<BufferSize> mBufferSizes;
    };

public:
    ShaderProgram(const std::string& vertexFile, const std::string& fragmentFile) {}
    virtual ~ShaderProgram() = default;

    BufferEnabler bufferEnabler(engine::graphics::Buffer& buffer, uint32_t startIndex = 0);

    virtual void bind() = 0;
    virtual void release() = 0;

    virtual void setAttributeBuffer(uint32_t index, uint32_t type, uint32_t offset, uint8_t count, uint32_t stride, bool divisor) = 0;

    virtual void setUniform(const std::string& name, const engine::graphics::Matrix& value) = 0;
    virtual void setUniform(const std::string& name, const engine::graphics::Vector<2>& value) = 0;
    virtual void setUniform(const std::string& name, const engine::graphics::Vector<4>& value) = 0;
    virtual void setUniform(const std::string& name, bool value) = 0;
    virtual void setUniform(const std::string& name, float value) = 0;
    virtual void setUniform(const std::string& name, int value) = 0;
    virtual void setUniform(const std::string& name, uint32_t value) = 0;

    virtual uint32_t typeSize(uint32_t type) = 0;

};
}
}
