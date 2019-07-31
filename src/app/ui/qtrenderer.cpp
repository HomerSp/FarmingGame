#include <QByteArray>
#include <QDataStream>
#include <QFontDatabase>
#include <QIODevice>
#include <QImage>

#include <engine/fontmanager.h>
#include <engine/logger.h>

#include <ui/qtrenderer.h>

QtRenderer::QtRenderer()
    : mPainter(nullptr)
{
    std::vector<std::string> fonts;
    if (engine::FontManager::get()->files(fonts)) {
        for (const std::string& f: fonts) {
            QFontDatabase::addApplicationFont(f.c_str());
        }
    }
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

    mPainter->setBrush(QColor(color.r, color.g, color.b, color.a));
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
    QColor c(color.r, color.g, color.b, color.a);
    mPainter->fillRect(dstRect, c);
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
    font.setFamily(engine::FontManager::get()->font(type).c_str());
    int32_t oldSize = font.pixelSize();
    if (size >= 0) {
        font.setPixelSize(size);
        mPainter->setFont(font);
    }

    mPainter->setPen({color.r, color.g, color.b, color.a});
    mPainter->drawText(QRect(dst.x, dst.y, dst.width, dst.height), flags, str);

    if (size >= 0) {
        font.setPixelSize(oldSize);
        mPainter->setFont(font);
    }
}

void QtRenderer::drawOverlay(const engine::Types::Point<>& dst, const engine::Types::Overlay& overlay)
{
    int32_t bufferSize = 0;
    for(auto& e: overlay.ellipses) {
        if (e.radius > bufferSize) {
            bufferSize = e.radius;
        }
    }

    QImage o(overlay.width + (bufferSize * 2), overlay.height + (bufferSize * 2), QImage::Format_ARGB32);
    o.fill(0);

    QPainter p(&o);
    p.translate(bufferSize, bufferSize);
    p.setPen(Qt::NoPen);
    p.fillRect(QRectF(0, 0, overlay.width, overlay.height), QColor(overlay.background.r, overlay.background.g, overlay.background.b, overlay.background.a));
    p.setCompositionMode(QPainter::CompositionMode_SourceIn);
    for(auto& e: overlay.ellipses) {
        QRadialGradient gradient(QPointF(e.radius / 2.0f, e.radius / 2.0f), e.radius / 2.0f, QPointF(e.radius / 2.0f, e.radius / 2.0f));
        gradient.setColorAt(0, QColor(e.color.r, e.color.g, e.color.b, e.color.a));
        gradient.setColorAt(1, QColor(overlay.background.r, overlay.background.g, overlay.background.b, 255));

        p.save();
        p.setBrush(gradient);
        p.translate(e.x - (e.radius / 2.0f), e.y - (e.radius / 2.0f));
        p.drawEllipse(0, 0, e.radius, e.radius);
        p.restore();
    }

    p.end();

    QRectF srcRect(bufferSize, bufferSize, overlay.width, overlay.height);
    QRectF dstRect(dst.x, dst.y, overlay.width, overlay.height);
    mPainter->drawImage(dstRect, o, srcRect);
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
    mPainter->setRenderHints(QPainter::Antialiasing | QPainter::TextAntialiasing | QPainter::SmoothPixmapTransform);

    QPainterPath screen;
    screen.addRect(0, 0, width(), height());
    mPainter->setClipPath(screen);

    QFont font = mPainter->font();
    font.setPixelSize(24);
    mPainter->setFont(font);
}

std::unique_ptr<engine::Image> QtRenderer::loadImage(const std::string& path)
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
