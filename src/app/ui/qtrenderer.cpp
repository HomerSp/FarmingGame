#include <QByteArray>
#include <QDataStream>
#include <QIODevice>
#include <QImage>

#include <engine/logger.h>

#include <ui/qtrenderer.h>

QtRenderer::QtRenderer()
    : mPainter(nullptr)
{
}

int QtRenderer::width()
{
    if (mPainter == nullptr) {
        engine::Logger::critical() << "No painter set!!!";
        return 0;
    }

    return mPainter->viewport().width();
}

int QtRenderer::height()
{
    if (mPainter == nullptr) {
        engine::Logger::critical() << "No painter set!!!";
        return 0;
    }

    return mPainter->viewport().height();
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

void QtRenderer::fillEllipse(const engine::Types::Rect<>& dst, const engine::Types::Color& fromColor, const engine::Types::Color& toColor)
{
    if (mPainter == nullptr) {
        engine::Logger::critical() << "No painter set!!!";
        return;
    }

    QRadialGradient gradient(dst.width / 2, dst.height / 2, dst.height / 2);
    gradient.setColorAt(0, QColor(fromColor.r, fromColor.g, fromColor.b, fromColor.a));
    gradient.setColorAt(1, QColor(toColor.r, toColor.g, toColor.b, toColor.a));

    QPainterPath path;
    path.addEllipse(QPointF(dst.width / 2, dst.height / 2), dst.width / 2, dst.height / 2);
    mPainter->translate(dst.x - (dst.width / 2), dst.y - (dst.height / 2));
    mPainter->fillPath(path, gradient);
    mPainter->translate(-(dst.x - (dst.width / 2)), -(dst.y - (dst.height / 2)));
}

void QtRenderer::drawImage(const engine::Image& img, const engine::Types::Rect<>& src, const engine::Types::Rect<>& dst)
{
    if (mPainter == nullptr) {
        engine::Logger::critical() << "No painter set!!!";
        return;
    }

    const QtImage& native = dynamic_cast<const QtImage&>(img);
    QRectF srcRect(src.x, src.y, src.width, src.height);
    QRectF dstRect(dst.x, dst.y, dst.width, dst.height);
    mPainter->drawImage(srcRect, native.image(), dstRect);
}

void QtRenderer::drawText(const engine::Types::Point<>& dst, const std::string& text, const engine::Types::Color& color, int size, engine::Types::TextAlign align)
{
    if (mPainter == nullptr) {
        engine::Logger::critical() << "No painter set!!!";
        return;
    }

    int flags = 0;
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
    int oldSize = font.pointSize();
    if (size >= 0) {
        font.setPointSize(size);
        mPainter->setFont(font);
    }

    mPainter->setPen({color.r, color.g, color.b, color.a});
    mPainter->drawText(QRect(dst.x, dst.y, width(), height()), flags, str);

    if (size >= 0) {
        font.setPointSize(oldSize);
        mPainter->setFont(font);
    }
}

void QtRenderer::translate(float x, float y)
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

void QtRenderer::eraseEllipses(const std::vector<engine::Types::Rect<>>& dst)
{
    if (mPainter == nullptr) {
        engine::Logger::critical() << "No painter set!!!";
        return;
    }

    QPainterPath screen;
    screen.addRect(0, 0, width(), height());

    QPainterPath clipped;
    for (auto& i: dst) {
        clipped.addEllipse(i.x - (i.width / 2), i.y - (i.height / 2), i.width, i.height);
    }

    mPainter->setClipPath(screen.subtracted(clipped));
}

 void QtRenderer::setPainter(QPainter* painter)
{
    mPainter = painter;

    QPainterPath screen;
    screen.addRect(0, 0, width(), height());
    mPainter->setClipPath(screen);
}

std::shared_ptr<engine::Image> QtRenderer::nativeImage(const std::string& path)
{
    return std::make_shared<QtImage>(path);
}

QtRenderer::QtImage::QtImage(const std::string& path)
    : Image()
{
    mImage = std::make_shared<QImage>(QString(path.c_str()));
}

const QImage& QtRenderer::QtImage::image() const
{
    return *mImage.get();
}

uint32_t QtRenderer::QtImage::width() const
{
    return mImage->width();
}

uint32_t QtRenderer::QtImage::height() const
{
    return mImage->height();
}