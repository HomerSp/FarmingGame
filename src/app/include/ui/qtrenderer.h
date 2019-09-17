#pragma once

#include <QOpenGLExtraFunctions>
#include <QOpenGLFramebufferObject>
#include <QOpenGLPaintDevice>
#include <QOpenGLShaderProgram>
#include <QOpenGLTexture>
#include <QOpenGLVertexArrayObject>
#include <QPainter>

#include <memory>

#include <engine/engine.h>
#include <engine/graphics/buffer.h>
#include <engine/graphics/image.h>
#include <engine/graphics/matrix.h>
#include <engine/graphics/texture.h>
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

    void beginNative() override;
    void endNative() override;

    void fillEllipse(const engine::Types::Rect<>& dst, const engine::graphics::Color& color) override;
    void fillRect(const engine::Types::Rect<>& dst, const engine::graphics::Color& color) override;

    void drawImage(const engine::graphics::Image& img, engine::Types::Rect<> dst, engine::Types::Rect<> src) override;
    void drawText(const engine::Types::Rect<>& dst, const std::string& text, const engine::graphics::Color& color, int32_t size, engine::Types::TextAlign align, std::string type) override;
    void drawOverlay(const engine::Types::Point<>& dst, engine::Overlay& overlay, uint32_t lightsCount, float mod) override;
    void drawParticles(const engine::Types::Point<>& dst, const engine::Particles& particles) override;
    void drawTexture(const engine::Types::Point<>& dst, engine::graphics::Texture* texture, engine::graphics::Buffer* buffer, uint32_t count) override;
    void drawTextureAnim(const engine::Types::Point<>& dst, engine::graphics::Texture* texture, engine::graphics::Buffer* buffer, float_t animFrame, uint32_t count) override;

    void rotate(float_t deg) override;
    void translate(int32_t x, int32_t y) override;

    void save() override;
    void restore() override;

    std::unique_ptr<engine::graphics::Buffer> createBuffer(uint32_t size) const override;
    std::unique_ptr<engine::graphics::Matrix> createMatrix() const override;
    std::unique_ptr<engine::graphics::Texture> createTexture(uint32_t w, uint32_t h, uint32_t layers) const override;
    std::unique_ptr<engine::graphics::Transform> createTransform() const override;

public slots:
    void cleanup();

protected:
    void initContext() override;

private:
    void setAttributeBuffer(int location, GLenum type, int offset, int tupleSize, int stride);

    std::unique_ptr<QOpenGLPaintDevice> mDevice;
    std::unique_ptr<QPainter> mPainter;

    std::unique_ptr<QOpenGLShaderProgram> mTexture2DShader, mTextureAnim2DShader;
    std::unique_ptr<QOpenGLShaderProgram> mPointLightShader;
    std::unique_ptr<QOpenGLShaderProgram> mColorShader;
    std::unique_ptr<QOpenGLShaderProgram> mParticle2DShader;
    std::unique_ptr<QOpenGLShaderProgram> mFBO2DShader;

    std::unique_ptr<QtMatrix> mWorldMatrix, mFBOMatrix;
    std::unique_ptr<QOpenGLFramebufferObject> mFBO;
    std::unique_ptr<QtBuffer> mQuadVertexBuffer, mCircleTextureBuffer;
    QOpenGLVertexArrayObject mOverlayVAO;
};
