#ifndef MYGRAPHICSVIEW_H
#define MYGRAPHICSVIEW_H

#include <QGraphicsView>
#include <QMouseEvent>
#include <QPointF>

class MyGraphicsView : public QGraphicsView
{
    Q_OBJECT

public:
    explicit MyGraphicsView(QWidget *parent = nullptr);

signals:
    void pointClicked(QPointF pos);  // 你在主窗口中要 connect 的信号

protected:
    void mousePressEvent(QMouseEvent *event) override;
};

#endif // MYGRAPHICSVIEW_H
