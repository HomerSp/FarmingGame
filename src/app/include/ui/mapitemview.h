#pragma once

#include <memory>

#include <QQuickPaintedItem>
#include <QPainter>
#include <QElapsedTimer>
#include <QKeyEvent>
#include <QTimer>

#include <engine/engine.h>
#include <engine/frametimer.h>
#include <engine/character.h>
#include <engine/keys.h>
#include <engine/map.h>

#include <ui/qtrenderer.h>

class MapItemView : public QQuickPaintedItem
{
	Q_OBJECT
public:
	MapItemView(QQuickItem* parent = 0);

	void paint(QPainter *painter) override;

signals:

protected:
	void focusInEvent(QFocusEvent *event) override;
	void focusOutEvent(QFocusEvent *event) override;
	void keyPressEvent(QKeyEvent *event) override;
	void keyReleaseEvent(QKeyEvent *event) override;
	void geometryChanged(const QRectF &newGeometry, const QRectF &oldGeometry);

private slots:
	void process();

private:
	std::shared_ptr<engine::Engine> mEngine;
	std::shared_ptr<QtRenderer> mRenderer;
	QTimer mIdleTimer;

};
