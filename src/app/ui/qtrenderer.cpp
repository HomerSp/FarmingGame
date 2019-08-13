#include <QByteArray>
#include <QDataStream>
#include <QFontDatabase>
#include <QIODevice>
#include <QImage>

#include <engine/context.h>
#include <engine/fontmanager.h>
#include <engine/logger.h>

#include <ui/qtrenderer.h>

QtRenderer::QtRenderer()
    : mPainter(nullptr)
    , mFBO(nullptr)
    , mSize(-1, -1)
{
}

void QtRenderer::init()
{
    std::vector<std::string> fonts;
    if (context().fontManager().files(fonts)) {
        for (const std::string& f: fonts) {
            QFontDatabase::addApplicationFont(f.c_str());
        }
    }
}

void QtRenderer::setSize(uint32_t w, uint32_t h)
{
    mSize.width = w;
    mSize.height = h;
}

void QtRenderer::paint(std::shared_ptr<engine::Engine>& engine, const engine::Types::Dimension<int32_t>& size, double pixelRatio)
{
    if (!mDevice) {
        mDevice = std::make_unique<QOpenGLPaintDevice>();
    }

    mDevice->setSize(QSize(size.width, size.height));
    mDevice->setDevicePixelRatio(pixelRatio);

    QPainter painter(mDevice.get());
    setPainter(&painter);
    engine->paint();
}

int32_t QtRenderer::width()
{
    if (mPainter == nullptr) {
        engine::Logger::critical() << "No painter set!!!";
        return 0;
    }

    return mPainter->viewport().width();
}

int32_t QtRenderer::height()
{
    if (mPainter == nullptr) {
        engine::Logger::critical() << "No painter set!!!";
        return 0;
    }

    return mPainter->viewport().height();
}

void QtRenderer::fillEllipse(const engine::Types::Rect<>& dst, const engine::Types::Color& color)
{
    if (mPainter == nullptr) {
        engine::Logger::critical() << "No painter set!!!";
        return;
    }

    mPainter->setBrush(QColor::fromRgbF(color.r, color.g, color.b, color.a));
    mPainter->drawEllipse(QPointF(dst.x, dst.y), std::floor(dst.width / 2), std::floor(dst.height / 2));
    mPainter->setBrush(Qt::NoBrush);
}

void QtRenderer::fillRect(const engine::Types::Rect<>& dst, const engine::Types::Color& color)
{
    if (mPainter == nullptr) {
        engine::Logger::critical() << "No painter set!!!";
        return;
    }

    QRectF dstRect(dst.x, dst.y, dst.width, dst.height);
    mPainter->fillRect(dstRect, QColor::fromRgbF(color.r, color.g, color.b, color.a));
}

void QtRenderer::drawImage(const engine::Image& img, engine::Types::Rect<> dst, engine::Types::Rect<> src)
{
    if (mPainter == nullptr) {
        engine::Logger::critical() << "No painter set!!!";
        return;
    }

    src.width = (src.width == 0) ? img.width() : src.width;
    src.height = (src.height == 0) ? img.height() : src.height;
    dst.width = (dst.width == 0) ? src.width : dst.width;
    dst.height = (dst.height == 0) ? src.height : dst.height;

    const auto& native = dynamic_cast<const QtImage&>(img);
    QRectF srcRect(src.x, src.y, src.width, src.height);
    QRectF dstRect(dst.x, dst.y, dst.width, dst.height);
    mPainter->drawImage(dstRect, native.image(), srcRect);
}

void QtRenderer::drawText(const engine::Types::Rect<>& dst, const std::string& text, const engine::Types::Color& color, int32_t size, engine::Types::TextAlign align, std::string type)
{
    if (mPainter == nullptr) {
        engine::Logger::critical() << "No painter set!!!";
        return;
    }

    int32_t flags = 0;
    if(align.is(engine::Types::TextAlign::Left)) {
        flags |= Qt::AlignLeft;
    } else if(align.is(engine::Types::TextAlign::Right)) {
        flags |= Qt::AlignRight;
    } else if(align.is(engine::Types::TextAlign::CentreH)) {
        flags |= Qt::AlignHCenter;
    }

    if(align.is(engine::Types::TextAlign::Top)) {
        flags |= Qt::AlignTop;
    } else if(align.is(engine::Types::TextAlign::Bottom)) {
        flags |= Qt::AlignBottom;
    } else if(align.is(engine::Types::TextAlign::CentreV)) {
        flags |= Qt::AlignVCenter;
    }

    QString str = QString(text.c_str());

    QFont font = mPainter->font();
    font.setFamily(context().fontManager().font(type).c_str());
    int32_t oldSize = font.pixelSize();
    if (size >= 0) {
        font.setPixelSize(size);
        mPainter->setFont(font);
    }

    mPainter->setPen(QColor::fromRgbF(color.r, color.g, color.b, color.a));
    mPainter->drawText(QRect(dst.x, dst.y, dst.width, dst.height), flags, str);

    if (size >= 0) {
        font.setPixelSize(oldSize);
        mPainter->setFont(font);
    }

    mPainter->setPen(Qt::NoPen);
}

void QtRenderer::drawOverlay(const engine::Types::Point<>& dst, const engine::Types::Overlay& overlay, float mod)
{
    mPainter->translate(dst.x, dst.y);
    mPainter->beginNativePainting();

    glViewport(0, 0, mPainter->viewport().width(), mPainter->viewport().height());
    glDisable(GL_DEPTH_TEST);

    mFBO->bind();

    glClearColor(overlay.background.r, overlay.background.g, overlay.background.b, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glBlendEquationSeparate(GL_FUNC_ADD, GL_MAX);

    for(auto& e: overlay.ellipses) {
        mPointLightShader->bind();

        engine::Types::Rect<float> vertexRect(e.x - (e.radius / 2.0f), e.y - (e.radius / 2.0f), e.radius, e.radius);

        std::array<QVector2D, 8> vertexPositions = {
            QVector2D(vertexRect.left(), vertexRect.top()), QVector2D(-1, -1),
            QVector2D(vertexRect.left(), vertexRect.bottom()), QVector2D(-1, 1),
            QVector2D(vertexRect.right(), vertexRect.top()), QVector2D(1, -1),
            QVector2D(vertexRect.right(), vertexRect.bottom()), QVector2D(1, 1)
        };

        mBufferCoords.bind();
        mBufferCoords.write(0, vertexPositions.data(), 8 * sizeof(QVector2D));

        mPointLightShader->enableAttributeArray(1);
        mPointLightShader->setAttributeBuffer(1, GL_FLOAT, 0, 2, sizeof(QVector2D) * 2);

        QVector4D inner(std::max(overlay.background.r, e.gradient.inner.r), std::max(overlay.background.g, e.gradient.inner.g), std::max(overlay.background.b, e.gradient.inner.b), 1.0f);
        QVector4D outer(std::max(overlay.background.r, e.gradient.outer.r), std::max(overlay.background.g, e.gradient.outer.g), std::max(overlay.background.b, e.gradient.outer.b), 1.0f);

        mPointLightShader->enableAttributeArray(0);
        mPointLightShader->setAttributeBuffer(0, GL_FLOAT, sizeof(QVector2D), 2, sizeof(QVector2D) * 2);
        mPointLightShader->setUniformValue("iMatrix", mMatrix);
        mPointLightShader->setUniformValue("iInnerColor", inner);
        mPointLightShader->setUniformValue("iOuterColor", outer);
        mPointLightShader->setUniformValue("iMod", std::abs(mod - 1.0f));

        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

        mPointLightShader->disableAttributeArray(0);
        mPointLightShader->disableAttributeArray(1);

        mBufferCoords.release();

        mPointLightShader->release();
    }

    mFBO->release();

    glEnable(GL_BLEND);
    glBlendFunc(GL_DST_COLOR, GL_ZERO);
    glBlendEquation(GL_FUNC_ADD);
    
    mTextureShader->bind();

    glBindTexture(GL_TEXTURE_2D, mFBO->texture());

    mBufferFBO.bind();

    mTextureShader->enableAttributeArray(1);
    mTextureShader->setAttributeBuffer(1, GL_FLOAT, 0, 2, sizeof(QVector2D) * 2);

    mTextureShader->enableAttributeArray(0);
    mTextureShader->setAttributeBuffer(0, GL_FLOAT, sizeof(QVector2D), 2, sizeof(QVector2D) * 2);
    mTextureShader->setUniformValue("iMatrix", mMatrix);
    mTextureShader->setUniformValue("iTexture", 0);

    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

    mTextureShader->disableAttributeArray(0);
    mTextureShader->disableAttributeArray(1);

    mBufferFBO.release();

    mTextureShader->release();

    mPainter->endNativePainting();
    mPainter->translate(-dst.x, -dst.y);
}

void QtRenderer::rotate(float_t deg)
{
    if (mPainter == nullptr) {
        engine::Logger::critical() << "No painter set!!!";
        return;
    }

    mPainter->rotate(deg);
}

void QtRenderer::translate(int32_t x, int32_t y)
{
    if (mPainter == nullptr) {
        engine::Logger::critical() << "No painter set!!!";
        return;
    }

    mPainter->translate(x, y);
}

void QtRenderer::save()
{
    if (mPainter == nullptr) {
        engine::Logger::critical() << "No painter set!!!";
        return;
    }

    mPainter->save();
}

void QtRenderer::restore()
{
    if (mPainter == nullptr) {
        engine::Logger::critical() << "No painter set!!!";
        return;
    }

    mPainter->restore();
}

void QtRenderer::setPainter(QPainter* painter)
{
    mPainter = painter;
    if (mPainter == nullptr) {
        return;
    }

    mPainter->setRenderHints(QPainter::Antialiasing | QPainter::TextAntialiasing | QPainter::SmoothPixmapTransform);

    QPainterPath screen;
    screen.addRect(0, 0, width(), height());
    mPainter->setClipPath(screen);

    QFont font = mPainter->font();
    font.setPixelSize(24);
    mPainter->setFont(font);

    mPainter->beginNativePainting();
    sync();
    mPainter->endNativePainting();
}

std::unique_ptr<engine::Image> QtRenderer::loadImage(const std::string& path) const
{
    return std::make_unique<QtImage>(path);
}

QtRenderer::QtImage::QtImage(const std::string& path)
{
    mImage = std::make_unique<QImage>(QString(path.c_str()));
}

const QImage& QtRenderer::QtImage::image() const
{
    return *mImage;
}

uint32_t QtRenderer::QtImage::width() const
{
    return mImage->width();
}

uint32_t QtRenderer::QtImage::height() const
{
    return mImage->height();
}

void QtRenderer::cleanup()
{
    mFBO.reset();
    mBufferCoords.destroy();
    mBufferFBO.destroy();
}

void QtRenderer::sync()
{
    if (!mFBO) {
        initializeOpenGLFunctions();

        mTextureShader = std::make_unique<QOpenGLShaderProgram>();
        mTextureShader->addShaderFromSourceFile(QOpenGLShader::Vertex, "assets/shader/vert_simple2d.glsl");
        mTextureShader->addShaderFromSourceFile(QOpenGLShader::Fragment, "assets/shader/frag_texture2d.glsl");
        mTextureShader->link();

        mPointLightShader = std::make_unique<QOpenGLShaderProgram>();
        mPointLightShader->addShaderFromSourceFile(QOpenGLShader::Vertex, "assets/shader/vert_simple2d.glsl");
        mPointLightShader->addShaderFromSourceFile(QOpenGLShader::Fragment, "assets/shader/frag_pointlight.glsl");
        mPointLightShader->link();

        mColorShader = std::make_unique<QOpenGLShaderProgram>();
        mColorShader->addShaderFromSourceFile(QOpenGLShader::Vertex, "assets/shader/vert_simple2d.glsl");
        mColorShader->addShaderFromSourceFile(QOpenGLShader::Fragment, "assets/shader/frag_color2d.glsl");
        mColorShader->link();

        mBufferCoords.create();
        mBufferCoords.bind();
        mBufferCoords.allocate(8 * sizeof(QVector2D));
        mBufferCoords.release();

        mBufferFBO.create();
        mBufferFBO.bind();
        mBufferFBO.allocate(8 * sizeof(QVector2D));
        mBufferFBO.release();
    }

    if (!mFBO || mSize.width >= 0) {
        mFBO = std::make_unique<QOpenGLFramebufferObject>(mPainter->viewport().width(), mPainter->viewport().height());
        mSize.width = mSize.height = -1;

        engine::Types::Rect<float> vertexRect(0, 0, mFBO->width(), mFBO->height());

        // The FBO texture is upside down (y starts at bottom), so we need to reverse it here
        std::array<QVector2D, 8> vertexPositions = {
            QVector2D(vertexRect.left(), vertexRect.top()), QVector2D(0, 1),      // Top left
            QVector2D(vertexRect.left(), vertexRect.bottom()), QVector2D(0, 0),   // Bottom left
            QVector2D(vertexRect.right(), vertexRect.top()), QVector2D(1, 1),     // Top right
            QVector2D(vertexRect.right(), vertexRect.bottom()), QVector2D(1, 0)   // Bottom right
        };

        mBufferFBO.bind();
        mBufferFBO.write(0, vertexPositions.data(), 8 * sizeof(QVector2D));
        mBufferFBO.release();

        QRect viewport = mPainter->viewport();
        mMatrix.ortho(0, viewport.width(), viewport.height(), 0, -1, 1);
    }
}