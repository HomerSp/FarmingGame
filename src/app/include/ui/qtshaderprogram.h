#pragma once

#include <cstdint>
#include <memory>

#include <QOpenGLExtraFunctions>
#include <QOpenGLShaderProgram>

#include <engine/graphics/shaderprogram.h>

class QtShaderProgram : public engine::graphics::ShaderProgram, protected QOpenGLExtraFunctions {
public:
    QtShaderProgram(const std::string& vertexFile, const std::string& fragmentFile);
    virtual ~QtShaderProgram() = default;

    virtual void bind() override;
    virtual void release() override;

    virtual void setAttributeBuffer(uint32_t index, uint32_t type, uint32_t offset, uint8_t count, uint32_t stride, bool divisor) override;

    virtual void setUniform(const std::string& name, const engine::graphics::Matrix& value) override;
    virtual void setUniform(const std::string& name, const engine::graphics::Vector<2>& value) override;
    virtual void setUniform(const std::string& name, const engine::graphics::Vector<4>& value) override;
    virtual void setUniform(const std::string& name, bool value) override;
    virtual void setUniform(const std::string& name, float value) override;
    virtual void setUniform(const std::string& name, int value) override;
    virtual void setUniform(const std::string& name, uint32_t value) override;

    virtual uint32_t typeSize(uint32_t type) override;

protected:
    int uniformLocation(const std::string& name);

private:
    QOpenGLShaderProgram mShaderProgram;
    std::map<std::string, int> mUniformLocations;
}; 
