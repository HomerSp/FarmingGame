#pragma once

#include <memory>

#include <QHoverEvent>
#include <QPainter>
#include <QQuickPaintedItem>

#include <engine/context.h>
#include <engine/map.h>
#include <engine/types.h>

#include <ui/qtrenderer.h>

class MapItemView : public QQuickPaintedItem {
    Q_OBJECT
public:
    MapItemView(QQuickItem* parent = 0);

    void paint(QPainter* painter) override;

protected:
    void hoverEnterEvent(QHoverEvent* event) override;
    void hoverLeaveEvent(QHoverEvent* event) override;
    void hoverMoveEvent(QHoverEvent* event) override;

private:
    QtRenderer mRenderer;
    std::shared_ptr<engine::Context> mContext;
    std::shared_ptr<engine::Map> mMap;

    engine::Types::Rect<> mSquare;
};
