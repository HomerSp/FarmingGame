#include <cmath>

#include <ui/mapitemview.h>

MapItemView::MapItemView(QQuickItem* parent)
	: QQuickPaintedItem(parent)
	, mMap(nullptr)
	, mSquare(-1, -1, 0, 0)
{
	setAcceptHoverEvents(true);

	mMap = std::make_shared<engine::Map>("map_small");
	setImplicitWidth(mMap->pixelWidth());
	setImplicitHeight(mMap->pixelHeight());

	engine::Types::Dimension squareDimen = mMap->getTileDimension();
	mSquare.width = squareDimen.width;
	mSquare.height = squareDimen.height;
}

void MapItemView::paint(QPainter *painter)
{
	mRenderer.setPainter(painter);

	engine::Types::Rect dst(0, 0, width(), height());
	mMap->draw(mRenderer, dst, false);

	if (mSquare.x >= 0)
	{
		mRenderer.fillRect(mSquare, engine::Types::Color(0, 0, 0, 50));

		engine::Types::Color lineColor(255, 255, 255);

		engine::Types::Rect left(mSquare.x, mSquare.y, 1, mSquare.height);
		mRenderer.fillRect(left, lineColor);

		engine::Types::Rect top(mSquare.x, mSquare.y, mSquare.width, 1);
		mRenderer.fillRect(top, lineColor);

		engine::Types::Rect bottom(mSquare.x, mSquare.y + mSquare.height - 1, mSquare.width, 1);
		mRenderer.fillRect(bottom, lineColor);

		engine::Types::Rect right(mSquare.x + mSquare.width - 1, mSquare.y, 1, mSquare.height);
		mRenderer.fillRect(right, lineColor);
	}
}

void MapItemView::hoverEnterEvent(QHoverEvent *event)
{
	Q_UNUSED(event);
	mSquare.x = 0;
	update();
}

void MapItemView::hoverLeaveEvent(QHoverEvent *event)
{
	Q_UNUSED(event);
	mSquare.x = -1;
	update();
}

void MapItemView::hoverMoveEvent(QHoverEvent *event)
{
	float x = std::floor(event->pos().x() / mSquare.width) * mSquare.width;
	float y = std::floor(event->pos().y() / mSquare.height) *  mSquare.height;
	if (x != mSquare.x || y != mSquare.y)
	{
		mSquare.x = x;
		mSquare.y = y;
		update();
	}
}