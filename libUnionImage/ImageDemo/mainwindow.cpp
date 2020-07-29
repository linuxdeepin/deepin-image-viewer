/*
* Copyright (C) 2019 ~ 2020 Deepin Technology Co., Ltd.
*
* Author: Deng jinhui<dengjinhui@uniontech.com>
*
* Maintainer: Deng jinhui <dengjinhui@uniontech.com>
*
* This program is free software: you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation, either version 3 of the License, or
* any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program. If not, see <http://www.gnu.org/licenses/>.
*/
#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    ui->centralWidget->setLayout(ui->verticalLayout);
    this->move(500, 200);
    //ui->otherinfolabel->setStyleSheet("");
    connect(ui->open_pushButton, &QPushButton::pressed, this, &MainWindow::on_pushButton_clicked);
    connect(&controler, &ImageDemoThreadControler::updateRotate, this, &MainWindow::updateRotateAngel);
    //connect(ui->leftrotate_pushButton, &QPushButton::pressed, this, &MainWindow::on_leftrotate_pushButton_pressed);
    //connect(ui->rightrotate_pushButton, &QPushButton::pressed, this, &MainWindow::on_rightrotate_pushButton_pressed);

    m_scence = new QGraphicsScene;
    ui->m_graphicsView->setScene(m_scence);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::updateRotateAngel(int angel)
{
    ui->rotate_label->setText(QString::number(angel));
}

void MainWindow::on_pushButton_clicked()
{
    QFileDialog dialog;
    dialog.setMimeTypeFilters(unionImageSupportFormat());
    //dialog.setOption(QFileDialog::HideNameFilterDetails);
    dialog.setWindowTitle(tr("Import Photos"));
    const QString path = dialog.getOpenFileName();
    QTime time;
    time.start();
    QImage image;
    QString errMsg;
    QString format = QFileInfo(path).suffix().toUtf8().toUpper();
    if (supportMovieFormat().contains(format)) {
        ici.setFileName(path);
        if (nullptr != dynamicImageTimer) {
            dynamicImageTimer->stop();
            delete dynamicImageTimer;
            dynamicImageTimer = nullptr;
        }
        dynamicImageTimer = new QTimer(this);
        connect(dynamicImageTimer, &QTimer::timeout, this, &MainWindow::loaddynamicImage);
        dynamicImageTimer->start(100);
        m_scence->clear();
        m_item = new QGraphicsPixmapItem;
        m_scence->addItem(m_item);
    } else {
        if (nullptr != dynamicImageTimer) {
            dynamicImageTimer->stop();
            delete dynamicImageTimer;
            dynamicImageTimer = nullptr;
        }
        if (!loadStaticImageFromFile(path, image, errMsg)) {

        }
        qDebug() << errMsg;
        //getThumbnail(image, path);
        qDebug() << "打开图片耗时：" << time.elapsed();
        currentImage = new QImage(image);
        currentImagePath = path;
        QSize size = ui->m_graphicsView->size();
        ui->filenamelabel->setText(path);
        m_scence->clear();
        m_item = new QGraphicsPixmapItem;
        m_scence->addItem(m_item);
        m_item->setPixmap(QPixmap::fromImage(image).scaled(size.width(), size.height(), Qt::IgnoreAspectRatio, Qt::SmoothTransformation));
        qDebug() << image;
    }

    QMap<QString, QString> Datas = getAllMetaData(path);
    QString str;
    for (auto data : Datas) {
        str += data;
        str += " ";
    }
    ui->otherinfolabel->setGeometry(QRect(328, 240, 329, 27 * 4)); //四倍行距
    ui->otherinfolabel->setWordWrap(true);
    ui->otherinfolabel->setAlignment(Qt::AlignTop);
    ui->otherinfolabel->setText(str);
}

void MainWindow::on_leftrotate_pushButton_pressed()
{
    QTime time;
    if (nullptr == currentImage) {
        return;
    }
    time.start();
    QImage image = QImage(*currentImage);
    RotateSaveRequest request;
    request.path = currentImagePath;
    request.angel = -90;
    request.image = image;
    controler.addRotateAndSave(request, 1000);
    rotateImage(-90, image);
    delete currentImage;
    currentImage = new QImage(image);
    QSize size = ui->m_graphicsView->size();
    m_item->setPixmap(QPixmap::fromImage(image).scaled(size.width() - 5, size.height() - 5, Qt::IgnoreAspectRatio, Qt::SmoothTransformation));
    qDebug() << "本次旋转耗时" << time.elapsed();
}

void MainWindow::on_rightrotate_pushButton_pressed()
{
    QTime time;
    if (nullptr == currentImage) {
        return;
    }
    time.start();
    QImage image = QImage(*currentImage);
    RotateSaveRequest request;
    request.path = currentImagePath;
    request.angel = 90;
    request.image = image;
    //对文件的旋转操作
    controler.addRotateAndSave(request, 1000);
    rotateImage(90, image);
    delete currentImage;
    currentImage = new QImage(image);
    QSize size = ui->m_graphicsView->size();
    m_item->setPixmap(QPixmap::fromImage(image).scaled(size.width() - 5, size.height() - 5, Qt::IgnoreAspectRatio, Qt::SmoothTransformation));
    qDebug() << "本次旋转耗时" << time.elapsed();
}

void MainWindow::loaddynamicImage()
{
    m_item->setPixmap(QPixmap::fromImage(ici++));
}
