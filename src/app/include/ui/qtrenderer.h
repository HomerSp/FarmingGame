#pragma once

#include <QOpenGLExtraFunctions>
#include <QOpenGLFramebufferObject>
#include <QOpenGLPaintDevice>
#include <QOpenGLShaderProgram>
#include <QPainter>

#include <memory>

#include <engine/engine.h>
#include <engine/graphics/buffer.h>
#include <engine/graphics/image.h>
#include <engine/graphics/matrix.h>
#include <engine/graphics/transform.h>
#include <engine/graphics/renderer.h>
#include <engine/overlay.h>
#include <engine/particles.h>

#include <ui/qtbuffer.h>
#include <ui/qtmatrix.h>

class QtRenderer : public QObject, public engine::graphics::Renderer, protected QOpenGLExtraFunctions
{
    Q_OBJECT
public:
    QtRenderer();
    virtual ~QtRenderer() = default;

    void setSize(uint32_t w, uint32_t h, double devicePixelRatio) override;

    void paint(std::shared_ptr<engine::Engine>& engine) override;

    int32_t width() override;
    int32_t height() override;

    void fillEllipse(const engine::Types::Rect<>& dst, const engine::graphics::Color& color) override;
    void fillRect(const engine::Types::Rect<>& dst, const engine::graphics::Color& color) override;

    void drawImage(const engine::graphics::Image& img, engine::Types::Rect<> dst, engine::Types::Rect<> src) override;
    void drawText(const engine::Types::Rect<>& dst, const std::string& text, const engine::graphics::Color& color, int32_t size, engine::Types::TextAlign align, std::string type) override;
    void drawOverlay(const engine::Types::Point<>& dst, engine::Overlay& overlay, uint32_t lightsCount, float mod) override;
    void drawParticles(const engine::Types::Point<>& dst, const engine::Particles& particles) override;

    void rotate(float_t deg) override;
    void translate(int32_t x, int32_t y) override;

    void save() override;
    void restore() override;

    std::unique_ptr<engine::graphics::Image> loadImage(const std::string& path) const override;

    std::unique_ptr<engine::graphics::Buffer> createBuffer(uint32_t size) const override;
    std::unique_ptr<engine::graphics::Matrix> createMatrix() const override;
    std::unique_ptr<engine::graphics::Transform> createTransform() const override;

public slots:
    void cleanup();

protected:
    void initContext() override;

private:
    std::unique_ptr<QOpenGLPaintDevice> mDevice;
    std::unique_ptr<QPainter> mPainter;
    
    std::unique_ptr<QOpenGLShaderProgram> mTextureShader;
    std::unique_ptr<QOpenGLShaderProgram> mPointLightShader;
    std::unique_ptr<QOpenGLShaderProgram> mColorShader;
    std::unique_ptr<QOpenGLShaderProgram> mParticleShader;

    std::unique_ptr<QtMatrix> mWorldMatrix, mProjectionMatrix, mFBOMatrix;
    std::unique_ptr<QOpenGLFramebufferObject> mFBO;
    std::unique_ptr<QtBuffer> mQuadVertexBuffer, mCircleTextureBuffer, mFBOTextureBuffer, mBufferMatrix;
};
