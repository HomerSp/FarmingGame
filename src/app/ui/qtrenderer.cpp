#include <QByteArray>
#include <QDataStream>
#include <QIODevice>
#include <QImage>

#include <ui/qtrenderer.h>

QtRenderer::QtRenderer()
	: mPainter(nullptr)
{

}

int QtRenderer::width()
{
	assert(mPainter != nullptr);
	return mPainter->viewport().width();
}

int QtRenderer::height()
{
	assert(mPainter != nullptr);
	return mPainter->viewport().height();
}

void QtRenderer::fillRect(const engine::Types::Rect& dst, const engine::Types::Color& color)
{
	assert(mPainter != nullptr);

	QRectF dstRect(dst.x, dst.y, dst.width, dst.height);
	QColor c(color.r, color.g, color.b, color.a);
	mPainter->fillRect(dstRect, c);
}

void QtRenderer::drawImage(const engine::Image& img, const engine::Types::Rect &src, const engine::Types::Rect &dst)
{
	assert(mPainter != nullptr);

	QImage i(img.data(), img.width(), img.height(), QImage::Format_ARGB32);
	QRectF srcRect(src.x, src.y, src.width, src.height);
	QRectF dstRect(dst.x, dst.y, dst.width, dst.height);
	mPainter->drawImage(srcRect, i, dstRect);
}

void QtRenderer::translate(float x, float y)
{
	assert(mPainter != nullptr);

	mPainter->translate(x, y);
}