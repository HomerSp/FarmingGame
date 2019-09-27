#include <QFontDatabase>
#include <QOpenGLPixelTransferOptions>
#include <QOpenGLVertexArrayObject>

#include <engine/context.h>
#include <engine/fontmanager.h>
#include <engine/graphics/quad.h>
#include <engine/graphics/textureloader.h>
#include <engine/graphics/vector.h>
#include <engine/logger.h>

#include <ui/qtrenderer.h>
#include <ui/qtshaderprogram.h>
#include <ui/qttexture.h>

QtRenderer::QtRenderer()
    : mPainter(nullptr)
    , mFBO(nullptr)
{
    initializeOpenGLFunctions();

    mDevice = std::make_unique<QOpenGLPaintDevice>();

    mWorldMatrix = std::make_unique<engine::graphics::Matrix>();
    mFBOMatrix = std::make_unique<engine::graphics::Matrix>();

    mQuadVertexBuffer = std::make_unique<QtBuffer>(engine::graphics::Quad2D::Size());

    mOverlayVAO.create();

    mQuadVertexBuffer->writer()
        .append(engine::graphics::Quad2D().lt(0, 0).lb(0, 1).rt(1, 0).rb(1, 1))
        .release();
}

void QtRenderer::setSize(uint32_t w, uint32_t h, double devicePixelRatio)
{
    mDevice->setSize(QSize(w * devicePixelRatio, h * devicePixelRatio));
    mDevice->setDevicePixelRatio(devicePixelRatio);

    mFBO = std::make_unique<QOpenGLFramebufferObject>(w, h);

    mWorldMatrix->reset();
    mWorldMatrix->ortho(0, w, 0, h, -1, 1);

    mFBOMatrix->reset();
    mFBOMatrix->scale(mFBO->width(), mFBO->height());

    QtBuffer overlayBuffer(engine::graphics::Vector4D::Size());
    overlayBuffer.writer()
        .append(engine::graphics::Vector4D(0, 0, mFBO->width(), mFBO->height()))
        .release();

    mOverlayVAO.bind();

    mFBO2DShader->bufferEnabler(*mQuadVertexBuffer)
        .append(GL_FLOAT, engine::graphics::Vector2D::Size())
        .buffer(overlayBuffer)
        .append(GL_FLOAT, engine::graphics::Vector4D::Size(), true)
        .release();

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
    /*if (mPainter == nullptr) {
        engine::Logger::critical("drawImage") << "No painter set!!!";
        return;
    }

    src.width = (src.width == 0) ? img.width() : src.width;
    src.height = (src.height == 0) ? img.height() : src.height;
    dst.width = (dst.width == 0) ? src.width : dst.width;
    dst.height = (dst.height == 0) ? src.height : dst.height;

    QImage img(img.data(), img.width(), img.height(), QImage::Format_RGBA8888);
    QRectF srcRect(src.x, src.y, src.width, src.height);
    QRectF dstRect(dst.x, dst.y, dst.width, dst.height);
    mPainter->drawImage(dstRect, img, srcRect);*/
}

void QtRenderer::drawText(const engine::Types::Rect<>& dst, const std::string& text, const engine::graphics::Color& color, int32_t size, engine::Types::TextAlign align, std::string type, bool shadow)
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

    if (shadow) {
        mPainter->setPen(QColor::fromRgbF(0, 0, 0, 0.1f));
        mPainter->drawText(QRect(dst.x + 1, dst.y + 1, dst.width + 1, dst.height + 1), flags, str);
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
    mWorldMatrix->translate(dst.x, dst.y);
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

    mPointLightShader->bufferEnabler(*mQuadVertexBuffer)
        .append(GL_FLOAT, engine::graphics::Vector2D::Size())
        .buffer(overlay.lightsBuffer())
        .append(GL_FLOAT, engine::graphics::Color::Size(), true)
        .append(GL_FLOAT, engine::graphics::Color::Size(), true)
        .append(GL_FLOAT, engine::graphics::Vector4D::Size(), true)
        .release();

    vao.release();

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glBlendEquationSeparate(GL_FUNC_ADD, GL_MAX);

    vao.bind();

    mPointLightShader->setUniform("uWorldMatrix", *mWorldMatrix);
    mPointLightShader->setUniform("iMod", std::abs(mod - 1.0f));

    glDrawArraysInstanced(GL_TRIANGLE_STRIP, 0, 4, lightsCount);

    vao.release();
    mPointLightShader->release();

    mFBO->release();
    mWorldMatrix->translate(-dst.x, -dst.y);

    mFBO2DShader->bind();
    mOverlayVAO.bind();

    glEnable(GL_BLEND);
    glBlendFunc(GL_DST_COLOR, GL_ZERO);
    glBlendEquation(GL_FUNC_ADD);

    glBindTexture(GL_TEXTURE_2D, mFBO->texture());

    mFBO2DShader->setUniform("uWorldMatrix", *mWorldMatrix);
    mFBO2DShader->setUniform("uTexture", 0);

    glDrawArraysInstanced(GL_TRIANGLE_STRIP, 0, 4, 1);

    mOverlayVAO.release();
    mFBO2DShader->release();
}

void QtRenderer::drawParticles(const engine::Types::Point<>& dst, const engine::Particles& particles, bool round)
{
    mWorldMatrix->translate(dst.x, dst.y);

    QOpenGLVertexArrayObject vao;
    vao.create();
    vao.bind();

    mParticle2DShader->bind();

    mParticle2DShader->bufferEnabler(*mQuadVertexBuffer)
        .append(GL_FLOAT, engine::graphics::Vector2D::Size())
        .buffer(particles.buffer())
        .append(GL_FLOAT, engine::graphics::Color::Size(), true)
        .append(GL_FLOAT, sizeof(float), true)
        .append(GL_FLOAT, engine::graphics::Vector2D::Size(), true)
        .append(GL_FLOAT, engine::graphics::Vector2D::Size(), true)
        .append(GL_FLOAT, engine::graphics::Vector2D::Size(), true)
        .release();

    vao.release();

    glDisable(GL_DEPTH_TEST);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    vao.bind();

    mParticle2DShader->setUniform("uWorldMatrix", *mWorldMatrix);
    mParticle2DShader->setUniform("uRound", round);

    glDrawArraysInstanced(GL_TRIANGLE_STRIP, 0, 4, particles.size());

    vao.release();

    mParticle2DShader->release();

    mWorldMatrix->translate(-dst.x, -dst.y);
}

void QtRenderer::drawTextures(const engine::Types::Point<>& dst, engine::graphics::Texture* texture, engine::graphics::Buffer* buffer, uint32_t count)
{
    mWorldMatrix->translate(dst.x, dst.y);

    QOpenGLVertexArrayObject vao;
    vao.create();
    vao.bind();

    uint32_t matrixStride = engine::graphics::Vector4D::Size() * 2 + sizeof(float_t);

    mTexture2DShader->bind();

    mTexture2DShader->bufferEnabler(*mQuadVertexBuffer)
        .append(GL_FLOAT, engine::graphics::Vector2D::Size())
        .buffer(*buffer)
        .append(GL_FLOAT, engine::graphics::Vector4D::Size(), true)
        .append(GL_FLOAT, engine::graphics::Vector4D::Size(), true)
        .append(GL_FLOAT, sizeof(float_t), true)
        .release();

    vao.release();

    glDisable(GL_DEPTH_TEST);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    vao.bind();
    texture->bind(0);

    mTexture2DShader->setUniform("uWorldMatrix", *mWorldMatrix);
    mTexture2DShader->setUniform("uTexture", 0);

    glDrawArraysInstanced(GL_TRIANGLE_STRIP, 0, 4, count);

    texture->release();
    vao.release();
    mTexture2DShader->release();

    mWorldMatrix->translate(-dst.x, -dst.y);
}

void QtRenderer::drawTexturesAnim(const engine::Types::Point<>& dst, engine::graphics::Texture* texture, engine::graphics::Buffer* buffer, float_t animFrame, uint32_t count)
{
    mWorldMatrix->translate(dst.x, dst.y);

    QOpenGLVertexArrayObject vao;
    vao.create();
    vao.bind();

    mTextureAnim2DShader->bind();

    mTextureAnim2DShader->bufferEnabler(*mQuadVertexBuffer)
        .append(GL_FLOAT, engine::graphics::Vector2D::Size())
        .buffer(*buffer)
        .append(GL_FLOAT, engine::graphics::Vector4D::Size(), true)
        .append(GL_FLOAT, sizeof(float_t), true)
        .append(GL_FLOAT, engine::graphics::Vector4D::Size(), true)
        .append(GL_FLOAT, engine::graphics::Vector2D::Size(), true)
        .append(GL_FLOAT, sizeof(float_t), true)
        .release();

    vao.release();

    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    vao.bind();
    texture->bind(0);

    mTextureAnim2DShader->setUniform("uWorldMatrix", *mWorldMatrix);
    mTextureAnim2DShader->setUniform("uTexture", 0);
    mTextureAnim2DShader->setUniform("uAnimFrame", animFrame);

    glDrawArraysInstanced(GL_TRIANGLE_STRIP, 0, 4, count);

    texture->release();
    vao.release();
    mTextureAnim2DShader->release();

    mWorldMatrix->translate(-dst.x, -dst.y);
}

void QtRenderer::drawTest()
{
    std::string text = "TESTing";

    QtBuffer buffer((engine::graphics::Vector3D::Size() + sizeof(uint32_t)) * text.size());
    auto writer = buffer.writer();

    float ratio = mTestTexture->width(0) / static_cast<float>(mTestTexture->height(0));
    float x = 100;
    for (auto c: text) {
        writer.append(engine::graphics::Vector3D(x, 100, 24))
            .append(static_cast<uint32_t>(c));

        x += 24.0 * ratio;
    }

    QOpenGLVertexArrayObject vao;
    vao.create();
    vao.bind();

    mTestShader->bind();

    mTestShader->bufferEnabler(*mQuadVertexBuffer)
        .append(GL_FLOAT, engine::graphics::Vector2D::Size())
        .buffer(buffer)
        .append(GL_FLOAT, engine::graphics::Vector3D::Size(), true)
        .append(GL_FLOAT, sizeof(uint32_t), true)
        .release();

    vao.release();

    glDisable(GL_DEPTH_TEST);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    vao.bind();
    mTestTexture->bind(0);

    mTestShader->setUniform("uWorldMatrix", *mWorldMatrix);
    mTestShader->setUniform("uCharSize", engine::graphics::Vector2D(mTestTexture->width(0) / 10.0f, mTestTexture->height(0) / 10.0f));
    mTestShader->setUniform("uTexture", 0);

    glDrawArraysInstanced(GL_TRIANGLE_STRIP, 0, 4, text.size());

    mTestTexture->release();
    vao.release();

    mTestShader->release();
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

std::unique_ptr<engine::graphics::Buffer> QtRenderer::createBuffer(uint32_t size) const
{
    return std::make_unique<QtBuffer>(size);
}

std::unique_ptr<engine::graphics::ShaderProgram> QtRenderer::createShader(const std::string& vertexFile, const std::string& fragmentFile) const
{
    return std::make_unique<QtShaderProgram>(vertexFile, fragmentFile);
}

std::unique_ptr<engine::graphics::Texture> QtRenderer::createTexture(uint32_t w, uint32_t h, uint32_t layers) const
{
    return std::make_unique<QtTexture>(w, h, layers);
}

void QtRenderer::cleanup()
{
    mFBO.reset();
}

void QtRenderer::initContext()
{
    std::vector<std::string> fonts;
    if (context().fontManager().files(fonts)) {
        for (const auto& f: fonts) {
            QFontDatabase::addApplicationFont(f.c_str());
        }
    }

    auto& am = context().assetManager();
    mFBO2DShader = am.shader(*this, "fbo2d", "fbo2d");
    mTexture2DShader = am.shader(*this, "texture2d", "texture2d");
    mTextureAnim2DShader = am.shader(*this, "textureanim2d", "textureanim2d");
    mPointLightShader = am.shader(*this, "pointlight", "pointlight");
    mColorShader = am.shader(*this, "color2d", "color2d");
    mParticle2DShader = am.shader(*this, "particle2d", "particle2d");

    mTestShader = am.shader(*this, "test", "test");

    engine::graphics::TextureLoader loader(*this);
    loader += *am.image(engine::AssetManager::Ui, "font-regular");
    loader += *am.image(engine::AssetManager::Ui, "font-bold");
    loader += *am.image(engine::AssetManager::Ui, "font-italic");
    loader.finish(mTestTexture);
}
