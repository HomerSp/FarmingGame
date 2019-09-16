#include <QFontDatabase>
#include <QOpenGLPixelTransferOptions>
#include <QOpenGLVertexArrayObject>

#include <engine/context.h>
#include <engine/fontmanager.h>
#include <engine/graphics/bufferwriter.h>
#include <engine/graphics/quad.h>
#include <engine/graphics/vector.h>
#include <engine/logger.h>

#include <ui/qtimage.h>
#include <ui/qtmatrix.h>
#include <ui/qtrenderer.h>
#include <ui/qttexture.h>
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

    mParticle2DShader = std::make_unique<QOpenGLShaderProgram>();
    mParticle2DShader->addShaderFromSourceFile(QOpenGLShader::Vertex, "assets/shader/particle2d.vs");
    mParticle2DShader->addShaderFromSourceFile(QOpenGLShader::Fragment, "assets/shader/particle2d.fs");
    mParticle2DShader->link();

    mCharset2DShader = std::make_unique<QOpenGLShaderProgram>();
    mCharset2DShader->addShaderFromSourceFile(QOpenGLShader::Vertex, "assets/shader/charset2d.vs");
    mCharset2DShader->addShaderFromSourceFile(QOpenGLShader::Fragment, "assets/shader/charset2d.fs");
    mCharset2DShader->link();

    mTiles2DShader = std::make_unique<QOpenGLShaderProgram>();
    mTiles2DShader->addShaderFromSourceFile(QOpenGLShader::Vertex, "assets/shader/tiles2d.vs");
    mTiles2DShader->addShaderFromSourceFile(QOpenGLShader::Fragment, "assets/shader/tiles2d.fs");
    mTiles2DShader->link();

    mWorldMatrix = std::make_unique<QtMatrix>();
    mFBOMatrix = std::make_unique<QtMatrix>();

    mQuadVertexBuffer = std::make_unique<QtBuffer>(engine::graphics::Quad2D::Size());
    mCircleTextureBuffer = std::make_unique<QtBuffer>(engine::graphics::Quad2D::Size());

    mOverlayVAO.create();

    engine::graphics::BufferWriter quadWriter(*mQuadVertexBuffer);
    quadWriter += engine::graphics::Quad2D().lt(0, 0).lb(0, 1).rt(1, 0).rb(1, 1);
    quadWriter.release();

    engine::graphics::BufferWriter vboWriter(*mCircleTextureBuffer);
    vboWriter += engine::graphics::Quad2D().lt(-1, -1).lb(-1, 1).rt(1, -1).rb(1, 1);
    vboWriter.release();
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

    QtBuffer overlayBuffer(engine::graphics::Vector4D::Size() * 2);
    engine::graphics::BufferWriter fboWriter(overlayBuffer);
    fboWriter += engine::graphics::Vector4D(0, 0, mFBO->width(), mFBO->height());
    fboWriter += engine::graphics::Vector4D(0, 0, mFBO->width(), mFBO->height());
    fboWriter.release();

    mOverlayVAO.bind();

    mQuadVertexBuffer->bind();
    mTextureShader->enableAttributeArray(0);
    mTextureShader->setAttributeBuffer(0, GL_FLOAT, 0, 2, engine::graphics::Vector2D::Size());
    mQuadVertexBuffer->release();

    uint32_t stride = engine::graphics::Vector2D::Size() + engine::graphics::Vector4D::Size();

    overlayBuffer.bind();
    mTextureShader->enableAttributeArray(1);
    mTextureShader->setAttributeBuffer(1, GL_FLOAT, 0, 4, stride);
    glVertexAttribDivisor(1, 1);

    mTextureShader->enableAttributeArray(2);
    mTextureShader->setAttributeBuffer(2, GL_FLOAT, engine::graphics::Vector4D::Size(), 4, stride);
    glVertexAttribDivisor(2, 1);
    overlayBuffer.release();
    mOverlayVAO.release();
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

void QtRenderer::beginNative()
{
    mPainter->beginNativePainting();
}

void QtRenderer::endNative()
{
    mPainter->endNativePainting();
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
    return;

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
    mFBO->bind();

    glViewport(0, 0, mFBO->width(), mFBO->height());
    glDisable(GL_DEPTH_TEST);

    const auto& overlayBackground = overlay.background();
    glClearColor(overlayBackground.r(), overlayBackground.g(), overlayBackground.b(), 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    QOpenGLVertexArrayObject vao;
    vao.create();
    vao.bind();

    mPointLightShader->bind();

    mCircleTextureBuffer->bind();
    mPointLightShader->enableAttributeArray(0);
    mPointLightShader->setAttributeBuffer(0, GL_FLOAT, 0, 2, engine::graphics::Vector2D::Size());
    mCircleTextureBuffer->release();

    mQuadVertexBuffer->bind();
    mPointLightShader->enableAttributeArray(1);
    mPointLightShader->setAttributeBuffer(1, GL_FLOAT, 0, 2, engine::graphics::Vector2D::Size());
    mQuadVertexBuffer->release();

    auto& overlayBuffer = overlay.buffer();
    overlayBuffer.bind();
    mPointLightShader->enableAttributeArray(2);
    mPointLightShader->setAttributeBuffer(2, GL_FLOAT, 0, 4, engine::graphics::ColorGradient::Size() + engine::graphics::Matrix::Size());
    glVertexAttribDivisor(2, 1);

    mPointLightShader->enableAttributeArray(3);
    mPointLightShader->setAttributeBuffer(3, GL_FLOAT, engine::graphics::Color::Size(), 4, engine::graphics::ColorGradient::Size() + engine::graphics::Matrix::Size());
    glVertexAttribDivisor(3, 1);

    for (uint32_t i = 0; i < 4; i++) {
        uint32_t offset = engine::graphics::ColorGradient::Size() + (engine::graphics::Vector4D::Size() * i);
        mPointLightShader->enableAttributeArray(4 + i);
        mPointLightShader->setAttributeBuffer(4 + i, GL_FLOAT, offset, 4, engine::graphics::ColorGradient::Size() + engine::graphics::Matrix::Size());
        glVertexAttribDivisor(4 + i, 1);
    }

    overlayBuffer.release();
    vao.release();

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glBlendEquationSeparate(GL_FUNC_ADD, GL_MAX);

    vao.bind();

    mPointLightShader->setUniformValue("uWorldMatrix", *mWorldMatrix);
    mPointLightShader->setUniformValue("iMod", std::abs(mod - 1.0f));

    glDrawArraysInstanced(GL_TRIANGLE_STRIP, 0, 4, lightsCount);

    vao.release();
    mPointLightShader->release();

    mFBO->release();

    mWorldMatrix->translate(dst.x, dst.y);
    mTextureShader->bind();
    mOverlayVAO.bind();

    glEnable(GL_BLEND);
    glBlendFunc(GL_DST_COLOR, GL_ZERO);
    glBlendEquation(GL_FUNC_ADD);

    glBindTexture(GL_TEXTURE_2D, mFBO->texture());

    mTextureShader->setUniformValue("uWorldMatrix", *mWorldMatrix);
    mTextureShader->setUniformValue("uReverseY", static_cast<GLuint>(true));
    mTextureShader->setUniformValue("uTexture", 0);

    glDrawArraysInstanced(GL_TRIANGLE_STRIP, 0, 4, 1);

    mOverlayVAO.release();
    mTextureShader->release();
    mWorldMatrix->translate(-dst.x, -dst.y);
}

void QtRenderer::drawParticles(const engine::Types::Point<>& dst, const engine::Particles& particles)
{
    mWorldMatrix->translate(dst.x, dst.y);

    QOpenGLVertexArrayObject vao;
    vao.create();
    vao.bind();

    mParticle2DShader->bind();

    mQuadVertexBuffer->bind();
    mParticle2DShader->enableAttributeArray(0);
    mParticle2DShader->setAttributeBuffer(0, GL_FLOAT, 0, 2, engine::graphics::Vector2D::Size());
    mQuadVertexBuffer->release();

    uint32_t stride = engine::graphics::Color::Size() + engine::graphics::Matrix::Size();
    auto& particleBuffer = particles.buffer();

    particleBuffer.bind();
    mParticle2DShader->enableAttributeArray(1);
    mParticle2DShader->setAttributeBuffer(1, GL_FLOAT, 0, 4, stride);
    glVertexAttribDivisor(1, 1);

    for (uint32_t i = 0; i < 4; i++) {
        mParticle2DShader->enableAttributeArray(2 + i);
        mParticle2DShader->setAttributeBuffer(2 + i, GL_FLOAT, engine::graphics::Color::Size() + i * engine::graphics::Vector4D::Size(), 4, stride);
        glVertexAttribDivisor(2 + i, 1);
    }
    particleBuffer.release();

    vao.release();

    glDisable(GL_DEPTH_TEST);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    vao.bind();

    mParticle2DShader->setUniformValue("uWorldMatrix", *mWorldMatrix);

    glDrawArraysInstanced(GL_TRIANGLE_STRIP, 0, 4, particles.size());

    vao.release();

    mParticle2DShader->release();

    mWorldMatrix->translate(-dst.x, -dst.y);
}

void QtRenderer::drawCharset(const engine::Types::Point<>& dst, engine::graphics::Texture* texture, engine::graphics::Buffer* buffer, float_t animFrame, uint32_t count)
{
    mWorldMatrix->translate(dst.x, dst.y);

    QOpenGLVertexArrayObject vao;
    vao.create();
    vao.bind();

    uint32_t matrixStride = engine::graphics::Vector3D::Size() + engine::graphics::Vector4D::Size() + engine::graphics::Vector2D::Size() + sizeof(float_t);

    mCharset2DShader->bind();

    mQuadVertexBuffer->bind();
    mCharset2DShader->enableAttributeArray(0);
    mCharset2DShader->setAttributeBuffer(0, GL_FLOAT, 0, 2, engine::graphics::Vector2D::Size());
    mQuadVertexBuffer->release();
    
    buffer->bind();
    mCharset2DShader->enableAttributeArray(1);
    mCharset2DShader->setAttributeBuffer(1, GL_FLOAT, 0, 3, matrixStride);
    glVertexAttribDivisor(1, 1);

    mCharset2DShader->enableAttributeArray(2);
    mCharset2DShader->setAttributeBuffer(2, GL_FLOAT, engine::graphics::Vector3D::Size(), 4, matrixStride);
    glVertexAttribDivisor(2, 1);

    mCharset2DShader->enableAttributeArray(3);
    mCharset2DShader->setAttributeBuffer(3, GL_FLOAT, engine::graphics::Vector3D::Size() + engine::graphics::Vector4D::Size(), 2, matrixStride);
    glVertexAttribDivisor(3, 1);

    mCharset2DShader->enableAttributeArray(4);
    mCharset2DShader->setAttributeBuffer(4, GL_FLOAT, engine::graphics::Vector3D::Size() + engine::graphics::Vector4D::Size() + engine::graphics::Vector2D::Size(), 1, matrixStride);
    glVertexAttribDivisor(4, 1);
    buffer->release();

    vao.release();

    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    vao.bind();
    texture->bind(0);

    mCharset2DShader->setUniformValue("uWorldMatrix", *mWorldMatrix);
    mCharset2DShader->setUniformValue("uTexture", 0);
    mCharset2DShader->setUniformValue("uAnimFrame", animFrame);

    glDrawArraysInstanced(GL_TRIANGLE_STRIP, 0, 4, count);

    texture->release();
    vao.release();
    mCharset2DShader->release();

    mWorldMatrix->translate(-dst.x, -dst.y);
}

void QtRenderer::drawTiles(const engine::Types::Point<>& dst, engine::graphics::Texture* texture, engine::graphics::Buffer* buffer, float_t animFrame, uint32_t count)
{
    mWorldMatrix->translate(dst.x, dst.y);

    QOpenGLVertexArrayObject vao;
    vao.create();
    vao.bind();

    uint32_t matrixStride = engine::graphics::Vector3D::Size() + engine::graphics::Vector4D::Size() + engine::graphics::Vector2D::Size() + sizeof(float_t);

    mTiles2DShader->bind();

    mQuadVertexBuffer->bind();
    mTiles2DShader->enableAttributeArray(0);
    mTiles2DShader->setAttributeBuffer(0, GL_FLOAT, 0, 2, engine::graphics::Vector2D::Size());
    mQuadVertexBuffer->release();
    
    buffer->bind();
    mTiles2DShader->enableAttributeArray(1);
    mTiles2DShader->setAttributeBuffer(1, GL_FLOAT, 0, 3, matrixStride);
    glVertexAttribDivisor(1, 1);

    mTiles2DShader->enableAttributeArray(2);
    mTiles2DShader->setAttributeBuffer(2, GL_FLOAT, engine::graphics::Vector3D::Size(), 4, matrixStride);
    glVertexAttribDivisor(2, 1);

    mTiles2DShader->enableAttributeArray(3);
    mTiles2DShader->setAttributeBuffer(3, GL_FLOAT, engine::graphics::Vector3D::Size() + engine::graphics::Vector4D::Size(), 2, matrixStride);
    glVertexAttribDivisor(3, 1);

    mTiles2DShader->enableAttributeArray(4);
    mTiles2DShader->setAttributeBuffer(4, GL_FLOAT, engine::graphics::Vector3D::Size() + engine::graphics::Vector4D::Size() + engine::graphics::Vector2D::Size(), 1, matrixStride);
    glVertexAttribDivisor(4, 1);
    buffer->release();

    vao.release();

    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    vao.bind();
    texture->bind(0);

    mTiles2DShader->setUniformValue("uWorldMatrix", *mWorldMatrix);
    mTiles2DShader->setUniformValue("uTexture", 0);
    mTiles2DShader->setUniformValue("uAnimFrame", animFrame);

    glDrawArraysInstanced(GL_TRIANGLE_STRIP, 0, 4, count);

    texture->release();
    vao.release();
    mTiles2DShader->release();

    mWorldMatrix->translate(-dst.x, -dst.y);
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

    //mPainter->translate(x, y);
    //mWorldMatrix->translate(x, y);
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

std::unique_ptr<engine::graphics::Texture> QtRenderer::createTexture(uint32_t w, uint32_t h, uint32_t layers) const
{
    return std::make_unique<QtTexture>(w, h, layers);
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

void QtRenderer::setAttributeBuffer(int location, GLenum type, int offset, int tupleSize, int stride)
{
    glVertexAttribPointer(location, tupleSize, type, GL_FALSE, stride,
                              reinterpret_cast<const void *>(qintptr(offset)));
}