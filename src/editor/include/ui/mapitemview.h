#pragma once

#include <QHoverEvent>
#include <QQuickPaintedItem>
#include <QPainter>

#include <engine/types.h>
#include <engine/map.h>

#include <ui/qtrenderer.h>

class MapItemView : public QQuickPaintedItem
{
    Q_OBJECT
public:
	MapItemView(QQuickItem* parent = 0);

	void paint(QPainter *painter) override;

protected:
	void hoverEnterEvent(QHoverEvent *event);
	void hoverLeaveEvent(QHoverEvent *event);
	void hoverMoveEvent(QHoverEvent *event);

private:
	QtRenderer mRenderer;
	engine::Map* mMap;

	engine::Types::Rect mSquare;
};
