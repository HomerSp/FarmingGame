#pragma once

#include <QImage>
#include <QOpenGLBuffer>
#include <QOpenGLExtraFunctions>
#include <QOpenGLFramebufferObject>
#include <QOpenGLPaintDevice>
#include <QOpenGLShaderProgram>
#include <QOpenGLTexture>
#include <QOpenGLTextureBlitter>
#include <QPainter>

#include <memory>

#include <engine/engine.h>
#include <engine/graphics/buffer.h>
#include <engine/graphics/image.h>
#include <engine/graphics/matrix.h>
#include <engine/graphics/renderer.h>
#include <engine/particles.h>

#include <ui/qtbuffer.h>
#include <ui/qtmatrix.h>

class QtRenderer : public QObject, public engine::graphics::Renderer, protected QOpenGLExtraFunctions
{
    Q_OBJECT
public:
    QtRenderer();
    virtual ~QtRenderer() = default;

    void setSize(uint32_t w, uint32_t h, double devicePixelRatio);

    void paint(std::shared_ptr<engine::Engine>& engine);

    int32_t width();
    int32_t height();

    void fillEllipse(const engine::Types::Rect<>& dst, const engine::graphics::Color& color);
    void fillRect(const engine::Types::Rect<>& dst, const engine::graphics::Color& color);

    void drawImage(const engine::graphics::Image& img, engine::Types::Rect<> dst, engine::Types::Rect<> src);
    void drawText(const engine::Types::Rect<>& dst, const std::string& text, const engine::graphics::Color& color, int32_t size, engine::Types::TextAlign align, std::string type);
    void drawOverlay(const engine::Types::Point<>& dst, const engine::Types::Overlay& overlay, float mod);
    void drawParticles(const engine::Types::Point<>& dst, const engine::Particles& particles);

    void rotate(float_t deg);
    void translate(int32_t x, int32_t y);

    void save();
    void restore();

    std::unique_ptr<engine::graphics::Image> loadImage(const std::string& path) const;

public slots:
    void cleanup();

protected:
    void initContext();
    void sync();

private:
    std::unique_ptr<QOpenGLPaintDevice> mDevice;
    std::unique_ptr<QPainter> mPainter;
    
    std::unique_ptr<QOpenGLShaderProgram> mTextureShader;
    std::unique_ptr<QOpenGLShaderProgram> mPointLightShader;
    std::unique_ptr<QOpenGLShaderProgram> mColorShader;
    std::unique_ptr<QOpenGLShaderProgram> mParticleShader;

    std::unique_ptr<QtMatrix> mWorldMatrix, mProjectionMatrix, mFBOMatrix;
    std::unique_ptr<QOpenGLFramebufferObject> mFBO;
    std::unique_ptr<QtBuffer> mBufferVBO, mBufferFBO, mBufferMatrix;

    std::unique_ptr<QtBuffer> mLightsBuffer;
    std::unique_ptr<QtBuffer> mBufferParticles;

    engine::Types::Dimension<int32_t> mSize;
};
