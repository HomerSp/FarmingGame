#include <QByteArray>
#include <QDataStream>
#include <QFontDatabase>
#include <QIODevice>
#include <QImage>

#include <engine/context.h>
#include <engine/fontmanager.h>
#include <engine/graphics/bufferwriter.h>
#include <engine/graphics/vector.h>
#include <engine/graphics/vertex.h>
#include <engine/logger.h>

#include <ui/qtimage.h>
#include <ui/qtmatrix.h>
#include <ui/qtrenderer.h>
#include <ui/qttransform.h>

QtRenderer::QtRenderer()
    : mPainter(nullptr)
    , mFBO(nullptr)
{
    initializeOpenGLFunctions();

    mDevice = std::make_unique<QOpenGLPaintDevice>();

    mTextureShader = std::make_unique<QOpenGLShaderProgram>();
    mTextureShader->addShaderFromSourceFile(QOpenGLShader::Vertex, "assets/shader/texture2d.vs");
    mTextureShader->addShaderFromSourceFile(QOpenGLShader::Fragment, "assets/shader/texture2d.fs");
    mTextureShader->link();

    mPointLightShader = std::make_unique<QOpenGLShaderProgram>();
    mPointLightShader->addShaderFromSourceFile(QOpenGLShader::Vertex, "assets/shader/pointlight.vs");
    mPointLightShader->addShaderFromSourceFile(QOpenGLShader::Fragment, "assets/shader/pointlight.fs");
    mPointLightShader->link();

    mColorShader = std::make_unique<QOpenGLShaderProgram>();
    mColorShader->addShaderFromSourceFile(QOpenGLShader::Vertex, "assets/shader/color2d.vs");
    mColorShader->addShaderFromSourceFile(QOpenGLShader::Fragment, "assets/shader/color2d.fs");
    mColorShader->link();

    mParticleShader = std::make_unique<QOpenGLShaderProgram>();
    mParticleShader->addShaderFromSourceFile(QOpenGLShader::Vertex, "assets/shader/particle.vs");
    mParticleShader->addShaderFromSourceFile(QOpenGLShader::Fragment, "assets/shader/particle.fs");
    mParticleShader->link();

    mWorldMatrix = std::make_unique<QtMatrix>();
    mProjectionMatrix = std::make_unique<QtMatrix>();
    mFBOMatrix = std::make_unique<QtMatrix>();

    mQuadVertexBuffer = std::make_unique<QtBuffer>(engine::graphics::Vertex2D::Size);
    mCircleTextureBuffer = std::make_unique<QtBuffer>(engine::graphics::Vertex2D::Size);
    mFBOTextureBuffer = std::make_unique<QtBuffer>(engine::graphics::Vertex2D::Size);
    mBufferMatrix = std::make_unique<QtBuffer>(engine::graphics::Matrix::Size);

    engine::graphics::BufferWriter quadWriter(*mQuadVertexBuffer);
    quadWriter += engine::graphics::Vertex2D().tl(0, 0).bl(0, 1).tr(1, 0).br(1, 1);
    quadWriter.release();

    engine::graphics::BufferWriter vboWriter(*mCircleTextureBuffer);
    vboWriter += engine::graphics::Vertex2D().tl(-1, -1).bl(-1, 1).tr(1, -1).br(1, 1);
    vboWriter.release();

    // The FBO texture uses opengl coordinates where y starts at the bottom, so we need to reverse it here
    engine::graphics::BufferWriter fboWriter(*mFBOTextureBuffer);
    fboWriter += engine::graphics::Vertex2D().tl(0, 1).bl(0, 0).tr(1, 1).br(1, 0);
    fboWriter.release();
}

void QtRenderer::setSize(uint32_t w, uint32_t h, double devicePixelRatio)
{
    mDevice->setSize(QSize(w * devicePixelRatio, h * devicePixelRatio));
    mDevice->setDevicePixelRatio(devicePixelRatio);

    mFBO = std::make_unique<QOpenGLFramebufferObject>(w, h);

    mWorldMatrix->reset();
    mWorldMatrix->ortho(0, w, h, 0, -1, 1);

    mFBOMatrix->reset();
    mFBOMatrix->scale(mFBO->width(), mFBO->height());
}

void QtRenderer::paint(std::shared_ptr<engine::Engine>& engine)
{
    mPainter = std::make_unique<QPainter>(mDevice.get());
    mPainter->setRenderHints(QPainter::Antialiasing | QPainter::TextAntialiasing | QPainter::SmoothPixmapTransform);

    QPainterPath screen;
    screen.addRect(0, 0, width(), height());
    mPainter->setClipPath(screen);

    QFont font = mPainter->font();
    font.setPixelSize(24);
    mPainter->setFont(font);

    mProjectionMatrix->reset();

    engine->paint();
    mPainter.reset();
}

int32_t QtRenderer::width()
{
    return mFBO->width();
}

int32_t QtRenderer::height()
{
    return mFBO->height();
}

void QtRenderer::fillEllipse(const engine::Types::Rect<>& dst, const engine::graphics::Color& color)
{
    if (mPainter == nullptr) {
        engine::Logger::critical("QtRenderer::fillEllipse") << "No painter set!!!";
        return;
    }

    mPainter->setBrush(QColor::fromRgbF(color.r(), color.g(), color.b(), color.a()));
    mPainter->drawEllipse(QPointF(dst.x, dst.y), std::floor(dst.width / 2), std::floor(dst.height / 2));
    mPainter->setBrush(Qt::NoBrush);
}

void QtRenderer::fillRect(const engine::Types::Rect<>& dst, const engine::graphics::Color& color)
{
    if (mPainter == nullptr) {
        engine::Logger::critical("QtRenderer::fillRect") << "No painter set!!!";
        return;
    }

    QRectF dstRect(dst.x, dst.y, dst.width, dst.height);
    mPainter->fillRect(dstRect, QColor::fromRgbF(color.r(), color.g(), color.b(), color.a()));
}

void QtRenderer::drawImage(const engine::graphics::Image& img, engine::Types::Rect<> dst, engine::Types::Rect<> src)
{
    if (mPainter == nullptr) {
        engine::Logger::critical("drawImage") << "No painter set!!!";
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

void QtRenderer::drawText(const engine::Types::Rect<>& dst, const std::string& text, const engine::graphics::Color& color, int32_t size, engine::Types::TextAlign align, std::string type)
{
    if (mPainter == nullptr) {
        engine::Logger::critical("drawText") << "No painter set!!!";
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

    mPainter->setPen(QColor::fromRgbF(color.r(), color.g(), color.b(), color.a()));
    mPainter->drawText(QRect(dst.x, dst.y, dst.width, dst.height), flags, str);

    if (size >= 0) {
        font.setPixelSize(oldSize);
        mPainter->setFont(font);
    }

    mPainter->setPen(Qt::NoPen);
}

void QtRenderer::drawOverlay(const engine::Types::Point<>& dst, engine::Overlay& overlay, uint32_t lightsCount, float mod)
{
    mPainter->beginNativePainting();

    mFBO->bind();

    glViewport(0, 0, mFBO->width(), mFBO->height());
    glDisable(GL_DEPTH_TEST);

    const auto& overlayBackground = overlay.background();
    glClearColor(overlayBackground.r(), overlayBackground.g(), overlayBackground.b(), 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glBlendEquationSeparate(GL_FUNC_ADD, GL_MAX);

    mProjectionMatrix->translate(dst.x, dst.y);
    mPointLightShader->bind();

    mCircleTextureBuffer->bind();
    mPointLightShader->enableAttributeArray(0);
    mPointLightShader->setAttributeBuffer(0, GL_FLOAT, 0, 2, engine::graphics::Vector2D::Size);
    mCircleTextureBuffer->release();

    mQuadVertexBuffer->bind();
    mPointLightShader->enableAttributeArray(1);
    mPointLightShader->setAttributeBuffer(1, GL_FLOAT, 0, 2, engine::graphics::Vector2D::Size);
    mQuadVertexBuffer->release();

    auto& overlayBuffer = overlay.buffer();
    overlayBuffer.bind();
    mPointLightShader->enableAttributeArray(2);
    mPointLightShader->setAttributeBuffer(2, GL_FLOAT, 0, 4, engine::graphics::ColorGradient::Size + engine::graphics::Matrix::Size);
    glVertexAttribDivisor(2, 1);

    mPointLightShader->enableAttributeArray(3);
    mPointLightShader->setAttributeBuffer(3, GL_FLOAT, engine::graphics::Color::Size, 4, engine::graphics::ColorGradient::Size + engine::graphics::Matrix::Size);
    glVertexAttribDivisor(3, 1);

    for (uint32_t i = 0; i < 4; i++) {
        uint32_t offset = engine::graphics::ColorGradient::Size + (engine::graphics::Vector4D::Size * i);
        mPointLightShader->enableAttributeArray(4 + i);
        mPointLightShader->setAttributeBuffer(4 + i, GL_FLOAT, offset, 4, engine::graphics::ColorGradient::Size + engine::graphics::Matrix::Size);
        glVertexAttribDivisor(4 + i, 1);
    }

    overlayBuffer.release();

    mPointLightShader->setUniformValue("iWorldMatrix", *mWorldMatrix);
    mPointLightShader->setUniformValue("iProjectionMatrix", *mProjectionMatrix);
    mPointLightShader->setUniformValue("iMod", std::abs(mod - 1.0f));

    glDrawArraysInstanced(GL_TRIANGLE_STRIP, 0, 4, lightsCount);

    mPointLightShader->disableAttributeArray(0);
    mPointLightShader->disableAttributeArray(1);
    mPointLightShader->disableAttributeArray(2);
    glVertexAttribDivisor(2, 0);
    mPointLightShader->disableAttributeArray(3);
    glVertexAttribDivisor(3, 0);
    for (uint32_t i = 0; i < 4; i++) {
        mPointLightShader->disableAttributeArray(4 + i);
        glVertexAttribDivisor(4 + i, 0);
    }

    mPointLightShader->release();

    mFBO->release();

    engine::graphics::BufferWriter matrixWriter(*mBufferMatrix);
    matrixWriter += *mFBOMatrix;
    matrixWriter.release();

    glEnable(GL_BLEND);
    glBlendFunc(GL_DST_COLOR, GL_ZERO);
    glBlendEquation(GL_FUNC_ADD);
    
    mTextureShader->bind();

    glBindTexture(GL_TEXTURE_2D, mFBO->texture());

    mFBOTextureBuffer->bind();
    mTextureShader->enableAttributeArray(0);
    mTextureShader->setAttributeBuffer(0, GL_FLOAT, 0, 2, engine::graphics::Vector2D::Size);
    mFBOTextureBuffer->release();

    mQuadVertexBuffer->bind();
    mTextureShader->enableAttributeArray(1);
    mTextureShader->setAttributeBuffer(1, GL_FLOAT, 0, 2, engine::graphics::Vector2D::Size);
    mQuadVertexBuffer->release();

    mBufferMatrix->bind();
    for (uint32_t i = 0; i < 4; i++) {
        mTextureShader->enableAttributeArray(2 + i);
        mTextureShader->setAttributeBuffer(2 + i, GL_FLOAT, i * engine::graphics::Vector4D::Size, 4, engine::graphics::Matrix::Size);
        glVertexAttribDivisor(2 + i, 1);

        mTextureShader->enableAttributeArray(6 + i);
        mTextureShader->setAttributeBuffer(6 + i, GL_FLOAT, i * engine::graphics::Vector4D::Size, 4, engine::graphics::Matrix::Size);
        glVertexAttribDivisor(6 + i, 1);
    }

    mBufferMatrix->release();

    mTextureShader->setUniformValue("iWorldMatrix", *mWorldMatrix);
    mTextureShader->setUniformValue("iProjectionMatrix", *mProjectionMatrix);
    mTextureShader->setUniformValue("iTexture", 0);
    mTextureShader->setUniformValue("iResolution", QVector2D(mFBO->width(), mFBO->height()));

    glDrawArraysInstanced(GL_TRIANGLE_STRIP, 0, 4, 1);

    mTextureShader->disableAttributeArray(0);
    mTextureShader->disableAttributeArray(1);
    for (uint32_t i = 0; i < 4; i++) {
        mTextureShader->disableAttributeArray(2 + i);
        glVertexAttribDivisor(2 + i, 0);

        mTextureShader->disableAttributeArray(6 + i);
        glVertexAttribDivisor(6 + i, 0);
    }

    mTextureShader->release();
    mProjectionMatrix->translate(-dst.x, -dst.y);

    mPainter->endNativePainting();
}

void QtRenderer::drawParticles(const engine::Types::Point<>& dst, const engine::Particles& particles)
{
    mPainter->beginNativePainting();
    mProjectionMatrix->translate(dst.x, dst.y);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    mParticleShader->bind();

    mQuadVertexBuffer->bind();
    mParticleShader->enableAttributeArray(0);
    mParticleShader->setAttributeBuffer(0, GL_FLOAT, 0, 2, engine::graphics::Vector2D::Size);
    mQuadVertexBuffer->release();

    auto& particleBuffer = particles.buffer();
    particleBuffer.bind();
    mParticleShader->enableAttributeArray(1);
    mParticleShader->setAttributeBuffer(1, GL_FLOAT, 0, 4, engine::graphics::Color::Size + engine::graphics::Matrix::Size);
    glVertexAttribDivisor(1, 1);

    for (uint32_t i = 0; i < 4; i++) {
        uint32_t offset = engine::graphics::Color::Size + i * engine::graphics::Vector4D::Size;
        mParticleShader->enableAttributeArray(2 + i);
        mParticleShader->setAttributeBuffer(2 + i, GL_FLOAT, offset, 4, engine::graphics::Color::Size + engine::graphics::Matrix::Size);
        glVertexAttribDivisor(2 + i, 1);
    }

    particleBuffer.release();

    mParticleShader->setUniformValue("iWorldMatrix", *mWorldMatrix);
    mParticleShader->setUniformValue("iProjectionMatrix", *mProjectionMatrix);

    glDrawArraysInstanced(GL_TRIANGLE_STRIP, 0, 4, particles.size());

    mParticleShader->disableAttributeArray(0);
    mParticleShader->disableAttributeArray(1);
    glVertexAttribDivisor(1, 0);
    for (uint32_t i = 0; i < 4; i++) {
        mParticleShader->disableAttributeArray(2 + i);
        glVertexAttribDivisor(2 + i, 0);
    }

    mParticleShader->release();

    mProjectionMatrix->translate(-dst.x, -dst.y);
    mPainter->endNativePainting();
}

void QtRenderer::drawTest(float_t angle)
{
    mPainter->beginNativePainting();

    mColorShader->bind();

    mQuadVertexBuffer->bind();
    mColorShader->enableAttributeArray(0);
    mColorShader->setAttributeBuffer(0, GL_FLOAT, 0, 2, engine::graphics::Vector2D::Size);
    mQuadVertexBuffer->release();

    engine::graphics::Color color(1.0, 1.0, 1.0f, 1.0f);

    auto transform = createTransform();
    transform->translate(50, 50);
    transform->rotate(angle);
    transform->translate(-50, -50);
    transform->translate(100, 100);
    transform->scale(100, 2);

    auto buffer = createBuffer(engine::graphics::Color::Size + engine::graphics::Matrix::Size);
    engine::graphics::BufferWriter writer(*buffer);
    writer += color;
    writer += *transform;

    mColorShader->enableAttributeArray(1);
    mColorShader->setAttributeBuffer(1, GL_FLOAT, 0, 4, engine::graphics::Color::Size + engine::graphics::Matrix::Size);
    glVertexAttribDivisor(1, 1);

    for (uint32_t i = 0; i < 4; i++) {
        uint32_t offset = engine::graphics::Color::Size + i * engine::graphics::Vector4D::Size;
        mColorShader->enableAttributeArray(2 + i);
        mColorShader->setAttributeBuffer(2 + i, GL_FLOAT, offset, 4, engine::graphics::Color::Size + engine::graphics::Matrix::Size);
        glVertexAttribDivisor(2 + i, 1);
    }

    writer.release();

    mColorShader->setUniformValue("iWorldMatrix", *mWorldMatrix);
    mColorShader->setUniformValue("iProjectionMatrix", *mProjectionMatrix);

    glDrawArraysInstanced(GL_TRIANGLE_STRIP, 0, 4, 1);

    mColorShader->disableAttributeArray(0);
    mColorShader->disableAttributeArray(1);
    glVertexAttribDivisor(1, 0);
    for (uint32_t i = 0; i < 4; i++) {
        mColorShader->disableAttributeArray(2 + i);
        glVertexAttribDivisor(2 + i, 0);
    }

    mColorShader->release();

    mPainter->endNativePainting();
}

void QtRenderer::rotate(float_t deg)
{
    if (mPainter == nullptr) {
        engine::Logger::critical("QtRenderer::rotate") << "No painter set!!!";
        return;
    }

    mPainter->rotate(deg);
}

void QtRenderer::translate(int32_t x, int32_t y)
{
    if (mPainter == nullptr) {
        engine::Logger::critical("QtRenderer::translate") << "No painter set!!!";
        return;
    }

    mPainter->translate(x, y);
    //mProjectionMatrix->translate(x, y);
}

void QtRenderer::save()
{
    if (mPainter == nullptr) {
        engine::Logger::critical("QtRenderer::save") << "No painter set!!!";
        return;
    }

    mPainter->save();
}

void QtRenderer::restore()
{
    if (mPainter == nullptr) {
        engine::Logger::critical("QtRenderer::restore") << "No painter set!!!";
        return;
    }

    mPainter->restore();
}

std::unique_ptr<engine::graphics::Image> QtRenderer::loadImage(const std::string& path) const
{
    return std::make_unique<QtImage>(path);
}

std::unique_ptr<engine::graphics::Buffer> QtRenderer::createBuffer(uint32_t size) const
{
    return std::make_unique<QtBuffer>(size);
}

std::unique_ptr<engine::graphics::Matrix> QtRenderer::createMatrix() const
{
    return std::make_unique<QtMatrix>();
}

std::unique_ptr<engine::graphics::Transform> QtRenderer::createTransform() const
{
    return std::make_unique<QtTransform>();
}

void QtRenderer::cleanup()
{
    mFBO.reset();
}

void QtRenderer::initContext()
{
    std::vector<std::string> fonts;
    if (context().fontManager().files(fonts)) {
        for (const std::string& f: fonts) {
            QFontDatabase::addApplicationFont(f.c_str());
        }
    }
}
