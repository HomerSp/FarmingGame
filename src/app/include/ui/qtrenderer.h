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
#include <engine/image.h>
#include <engine/renderer.h>

class QtRenderer : public QObject, public engine::Renderer, protected QOpenGLExtraFunctions {
    Q_OBJECT
public:
    class QtImage : public engine::Image {
    public:
        QtImage(const std::string& path);
        virtual ~QtImage() = default;

        const QImage& image() const;

        virtual uint32_t width() const;
        virtual uint32_t height() const;

    private:
        std::unique_ptr<QImage> mImage;
    };

public:
    QtRenderer();
    virtual ~QtRenderer() = default;

    void init();
    void setSize(uint32_t w, uint32_t h);

    void paint(std::shared_ptr<engine::Engine>& engine, const engine::Types::Dimension<int32_t>& size, double pixelRatio);

    int32_t width();
    int32_t height();

    void fillEllipse(const engine::Types::Rect<>& dst, const engine::Types::Color& color);
    void fillRect(const engine::Types::Rect<>& dst, const engine::Types::Color& color);

    void drawImage(const engine::Image& img, engine::Types::Rect<> dst, engine::Types::Rect<> src);
    void drawText(const engine::Types::Rect<>& dst, const std::string& text, const engine::Types::Color& color, int32_t size, engine::Types::TextAlign align, std::string type);
    void drawOverlay(const engine::Types::Point<>& dst, const engine::Types::Overlay& overlay, float mod);

    void rotate(float_t deg);
    void translate(int32_t x, int32_t y);

    void save();
    void restore();

    void setPainter(QPainter* painter);

    std::unique_ptr<engine::Image> loadImage(const std::string& path) const;

public slots:
    void cleanup();

protected:
    void sync();

private:
    std::unique_ptr<QOpenGLPaintDevice> mDevice;
    QPainter* mPainter;
    
    std::unique_ptr<QOpenGLShaderProgram> mTextureShader;
    std::unique_ptr<QOpenGLShaderProgram> mPointLightShader;
    std::unique_ptr<QOpenGLShaderProgram> mColorShader;

    QMatrix4x4 mMatrix;
    std::unique_ptr<QOpenGLFramebufferObject> mFBO;
    QOpenGLBuffer mBufferFBO, mBufferCoords;

    engine::Types::Dimension<int32_t> mSize;
};
