#include <engine/graphics/matrix.h>

#include <ui/qtshaderprogram.h>

QtShaderProgram::QtShaderProgram(const std::string& vertexFile, const std::string& fragmentFile)
    : ShaderProgram(vertexFile, fragmentFile)
{
    initializeOpenGLFunctions();

    mShaderProgram.addShaderFromSourceFile(QOpenGLShader::Vertex, vertexFile.c_str());
    mShaderProgram.addShaderFromSourceFile(QOpenGLShader::Fragment, fragmentFile.c_str());
    mShaderProgram.link();
}

void QtShaderProgram::bind()
{
    mShaderProgram.bind();
}

void QtShaderProgram::release()
{
    mShaderProgram.release();
}

void QtShaderProgram::setAttributeBuffer(uint32_t index, uint32_t type, uint32_t offset, uint8_t count, uint32_t stride, bool divisor)
{
    mShaderProgram.enableAttributeArray(index);
    mShaderProgram.setAttributeBuffer(index, static_cast<GLenum>(type), offset, count, stride);
    if (divisor) {
        glVertexAttribDivisor(index, 1);
    }
}

void QtShaderProgram::setUniform(const std::string& name, const engine::graphics::Matrix& value)
{
    glUniformMatrix4fv(uniformLocation(name), 1, GL_FALSE, value.constData());
}

void QtShaderProgram::setUniform(const std::string& name, const engine::graphics::Vector<2>& value)
{
    glUniform2fv(uniformLocation(name), 1, value.constData());
}

void QtShaderProgram::setUniform(const std::string& name, const engine::graphics::Vector<4>& value)
{
    glUniform4fv(uniformLocation(name), 1, value.constData());
}

void QtShaderProgram::setUniform(const std::string& name, bool value)
{
    mShaderProgram.setUniformValue(uniformLocation(name), static_cast<int>(value));
}

void QtShaderProgram::setUniform(const std::string& name, float value)
{
    mShaderProgram.setUniformValue(uniformLocation(name), value);
}

void QtShaderProgram::setUniform(const std::string& name, int value)
{
    mShaderProgram.setUniformValue(uniformLocation(name), value);
}

void QtShaderProgram::setUniform(const std::string& name, uint32_t value)
{
    mShaderProgram.setUniformValue(uniformLocation(name), value);
}

uint32_t QtShaderProgram::typeSize(uint32_t type)
{
    static std::map<GLenum, uint32_t> sSizes = {
        {GL_BYTE, sizeof(int8_t)},
        {GL_UNSIGNED_BYTE, sizeof(uint8_t)},
        {GL_SHORT, sizeof(int16_t)},
        {GL_UNSIGNED_SHORT, sizeof(uint16_t)},
        {GL_INT, sizeof(int32_t)},
        {GL_UNSIGNED_INT, sizeof(uint32_t)},
        {GL_FLOAT, sizeof(float)},
        {GL_DOUBLE, sizeof(double)},
    };

    return sSizes[static_cast<GLenum>(type)];
}

int QtShaderProgram::uniformLocation(const std::string& name)
{
    if (mUniformLocations.find(name) == mUniformLocations.end()) {
        mUniformLocations[name] = mShaderProgram.uniformLocation(name.c_str());
    }

    return mUniformLocations[name];
}