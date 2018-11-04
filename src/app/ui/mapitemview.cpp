#include <QTimer>
#include <QThread>
#include <ui/mapitemview.h>

MapItemView::MapItemView(QQuickItem* parent)
	: QQuickPaintedItem(parent)
{
	setAcceptHoverEvents(true);
	setFocus(true);
	setClip(true);

	mEngine = std::shared_ptr<engine::Engine>(new engine::Engine(width(), height()));
	mRenderer = std::shared_ptr<QtRenderer>(new QtRenderer());

	setImplicitWidth(width() + mEngine->bufferWidth());
	setImplicitHeight(height() + mEngine->bufferHeight());

	std::unordered_map<int, engine::Keys::Type> keys;
	keys[Qt::Key_Up] = engine::Keys::Up;
	keys[Qt::Key_Down] = engine::Keys::Down;
	keys[Qt::Key_Left] = engine::Keys::Left;
	keys[Qt::Key_Right] = engine::Keys::Right;
	keys[Qt::Key_Shift] = engine::Keys::Run;
	keys[Qt::Key_Control] = engine::Keys::Walk;
	keys[Qt::Key_Q] = engine::Keys::Friction;
	keys[Qt::Key_S] = engine::Keys::Test;

	mEngine->setKeyMap(keys);

	connect(&mIdleTimer, &QTimer::timeout, this, &MapItemView::process);
	mIdleTimer.start(0);
}

void MapItemView::focusInEvent(QFocusEvent *event)
{
	Q_UNUSED(event);
	mEngine->setFocus(true);
}

void MapItemView::focusOutEvent(QFocusEvent *event)
{
	Q_UNUSED(event);
	mEngine->setFocus(false);
}

void MapItemView::keyPressEvent(QKeyEvent *event)
{
	mEngine->setKeyDown(event->key());
}

void MapItemView::keyReleaseEvent(QKeyEvent *event)
{
	mEngine->setKeyUp(event->key());
}

void MapItemView::geometryChanged(const QRectF &newGeometry, const QRectF &oldGeometry)
{
	QQuickItem::geometryChanged(newGeometry, oldGeometry);

	mEngine->setSize(newGeometry.width(), newGeometry.height());
	update();
}

void MapItemView::paint(QPainter *painter)
{
	mRenderer->setPainter(painter);
	mEngine->paint(*mRenderer);
}

void MapItemView::process()
{
	if(mEngine->process())
	{
		update();
	}
}
