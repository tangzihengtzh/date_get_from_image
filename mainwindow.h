#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

#include <QGraphicsPixmapItem>
#include <QFile>
#include <QImageReader>
#include <QMessageBox>

#include <QFileDialog>
#include <QFile>
#include <QTextStream>
#include <QMessageBox>



#include "mygraphicsview.h"

enum MarkState {
    None,
    Marking_Xmin,
    Marking_Ymin,
    Marking_Xmax,
    Marking_Ymax
};


QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void loadImage();
    void handleInputButton();
    void onPointClicked(QPointF point);
    void updateClickResDisplay();

    void startSampling();                       // 进入采样模式
    void exportPointsToTxt();                   // 导出点为 txt 文件


private:
    Ui::MainWindow *ui;

    QGraphicsScene *scene;

    MarkState currentState = None;
    QPointF clickedPoints[4];  // 存储四个相对坐标

    bool collectingPoints = false;              // 是否在采样模式
    QList<QPointF> sampledPoints;               // 采样到的坐标点



};
#endif // MAINWINDOW_H
