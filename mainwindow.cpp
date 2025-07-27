#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QGraphicsPixmapItem>
#include <QFile>
#include <QImageReader>
#include <QMessageBox>

#include <QFileDialog>
#include <QFile>
#include <QTextStream>
#include <QMessageBox>



#include "mygraphicsview.h"


MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    // 创建 scene 并绑定到 ImgView
    scene = new QGraphicsScene(this);
    ui->ImgView->setScene(scene);

    // 按钮点击连接槽函数
    connect(ui->openImg, &QPushButton::clicked, this, &MainWindow::loadImage);

    connect(ui->inputxmin, &QPushButton::clicked, this, &MainWindow::handleInputButton);
    connect(ui->inputymin, &QPushButton::clicked, this, &MainWindow::handleInputButton);
    connect(ui->inputxmax, &QPushButton::clicked, this, &MainWindow::handleInputButton);
    connect(ui->inputymax, &QPushButton::clicked, this, &MainWindow::handleInputButton);

    connect(ui->ImgView, &MyGraphicsView::pointClicked, this, &MainWindow::onPointClicked);

    connect(ui->getdata, &QPushButton::clicked, this, &MainWindow::startSampling);
    connect(ui->outputtxt, &QPushButton::clicked, this, &MainWindow::exportPointsToTxt);

    this->setWindowTitle("Get Points");

}

void MainWindow::loadImage()
{
    // 获取路径
    QString path = ui->ImagePath->toPlainText();

    if (!QFile::exists(path)) {
        QMessageBox::warning(this, "错误", "文件不存在！");
        return;
    }

    QImageReader reader(path);
    reader.setAutoTransform(true);
    QImage image = reader.read();
    if (image.isNull()) {
        QMessageBox::warning(this, "错误", "无法读取图像！");
        return;
    }

    QPixmap pixmap = QPixmap::fromImage(image);
    scene->clear();  // 清除旧图像

    // 添加图像到场景
    QGraphicsPixmapItem *item = scene->addPixmap(pixmap);
    item->setTransformationMode(Qt::SmoothTransformation);

    // 计算等比缩放尺寸
    QRectF viewRect = ui->ImgView->viewport()->rect();
    QRectF sceneRect = pixmap.rect();

    qreal scaleX = viewRect.width() / sceneRect.width();
    qreal scaleY = viewRect.height() / sceneRect.height();
    qreal scale = qMin(scaleX, scaleY);

    ui->ImgView->resetTransform();  // 重置任何已有缩放
    ui->ImgView->scale(scale, scale);  // 等比缩放
    ui->ImgView->centerOn(item);  // 居中显示
}

void MainWindow::handleInputButton()
{
    QPushButton* btn = qobject_cast<QPushButton*>(sender());
    if (btn == ui->inputxmin) currentState = Marking_Xmin;
    else if (btn == ui->inputymin) currentState = Marking_Ymin;
    else if (btn == ui->inputxmax) currentState = Marking_Xmax;
    else if (btn == ui->inputymax) currentState = Marking_Ymax;

    ui->currentState->setText("Now marking: " + btn->text());  // 状态更新
}

void MainWindow::onPointClicked(QPointF point)
{
    if (collectingPoints) {
        // 记录点击点
        sampledPoints.append(point);

        // 在 scene 上添加蓝色圆点标记采样点
        QPen pen(Qt::blue);
        QBrush brush(Qt::blue);
        scene->addEllipse(point.x() - 3, point.y() - 3, 6, 6, pen, brush);

        // 获取用户输入的绝对坐标值（注意对象名匹配你 UI 上的控件）
        double Xmin_input = ui->xmin->toPlainText().toDouble();
        double Xmax_input = ui->xmax->toPlainText().toDouble();
        double Ymin_input = ui->ymin->toPlainText().toDouble();
        double Ymax_input = ui->ymax->toPlainText().toDouble();

        // 获取四个标定参考点（图像坐标）
        QPointF pt_xmin = clickedPoints[0];
        QPointF pt_ymin = clickedPoints[1];
        QPointF pt_xmax = clickedPoints[2];
        QPointF pt_ymax = clickedPoints[3];

        // X方向比例计算（防止除以0）
        double x_ratio = 0.0;
        if (pt_xmax.x() != pt_xmin.x())
            x_ratio = (point.x() - pt_xmin.x()) / (pt_xmax.x() - pt_xmin.x());

        // Y方向比例计算
        double y_ratio = 0.0;
        if (pt_ymax.y() != pt_ymin.y())
            y_ratio = (point.y() - pt_ymin.y()) / (pt_ymax.y() - pt_ymin.y());

        // 映射到真实坐标系
        double X_real = Xmin_input + x_ratio * (Xmax_input - Xmin_input);
        double Y_real = Ymin_input + y_ratio * (Ymax_input - Ymin_input);

        // 显示结果到 userpoints 文本框
        ui->userpoints->append(QString("(%1, %2)")
                                   .arg(X_real, 0, 'f', 3)
                                   .arg(Y_real, 0, 'f', 3));

        return;  // 采样完成不执行后面的四点标定逻辑
    }

    // ====================
    // 以下为四点标定逻辑
    // ====================

    switch (currentState) {
    case Marking_Xmin:
        clickedPoints[0] = point;
        break;
    case Marking_Ymin:
        clickedPoints[1] = point;
        break;
    case Marking_Xmax:
        clickedPoints[2] = point;
        break;
    case Marking_Ymax:
        clickedPoints[3] = point;
        break;
    default:
        return;  // 未处于任何标定状态
    }

    // 添加红点表示标定点
    QPen pen(Qt::red);
    QBrush brush(Qt::red);
    scene->addEllipse(point.x() - 3, point.y() - 3, 6, 6, pen, brush);

    // 更新标定点显示
    updateClickResDisplay();

    // 状态清除
    currentState = None;
    ui->currentState->setText("Ready");
}


void MainWindow::updateClickResDisplay()
{
    QStringList lines;
    lines << "Xmin: " + QString("(%1, %2)").arg(clickedPoints[0].x()).arg(clickedPoints[0].y());
    lines << "Ymin: " + QString("(%1, %2)").arg(clickedPoints[1].x()).arg(clickedPoints[1].y());
    lines << "Xmax: " + QString("(%1, %2)").arg(clickedPoints[2].x()).arg(clickedPoints[2].y());
    lines << "Ymax: " + QString("(%1, %2)").arg(clickedPoints[3].x()).arg(clickedPoints[3].y());

    ui->clickres->setPlainText(lines.join("\n"));
}

void MainWindow::startSampling()
{
    collectingPoints = true;
    sampledPoints.clear();
    ui->userpoints->clear();
    ui->currentState->setText("Now sampling points: Click on the image...");
}

void MainWindow::exportPointsToTxt()
{
    QString fileName = QFileDialog::getSaveFileName(this, "Save Points", "", "Text Files (*.txt)");
    if (fileName.isEmpty())
        return;

    QFile file(fileName);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QMessageBox::warning(this, "Error", "Failed to open file for writing");
        return;
    }

    QTextStream out(&file);
    for (const QPointF& pt : sampledPoints) {
        out << pt.x() << ", " << pt.y() << "\n";
    }

    file.close();
    QMessageBox::information(this, "Success", "Points saved successfully!");
}
















































MainWindow::~MainWindow()
{
    delete ui;
}




