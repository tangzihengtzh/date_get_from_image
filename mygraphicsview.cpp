#include "mygraphicsview.h"

MyGraphicsView::MyGraphicsView(QWidget *parent)
    : QGraphicsView(parent)
{
}

void MyGraphicsView::mousePressEvent(QMouseEvent *event)
{
    QPointF scenePos = mapToScene(event->pos());
    emit pointClicked(scenePos);  // 发出你定义的信号
    QGraphicsView::mousePressEvent(event);  // 保留原有行为
}
