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
#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "imagedemothread.h"
#include "moviegraphicsitem.h"

#include <QGraphicsPixmapItem>
#include <QGraphicsView>
#include <QGraphicsScene>
#include <QMainWindow>
#include <QLabel>
#include <unionimage.h>
#include <QFileDialog>
#include <QDebug>
#include <QRunnable>
#include <QThreadPool>
#include <QPainter>
#include <QTime>


using namespace UnionImage_NameSpace;

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT
public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void updateRotateAngel(int angel);
    void loaddynamicImage();
    void on_pushButton_clicked();
    void on_leftrotate_pushButton_pressed();
    void on_rightrotate_pushButton_pressed();
private:
    UnionMovieImage ici;
    Ui::MainWindow *ui;
    QImage *currentImage = nullptr;
    QTimer *dynamicImageTimer = nullptr;
    QString currentImagePath;
    QGraphicsScene *m_scence;
    QGraphicsPixmapItem *m_item;
    MovieGraphicsItem *m_movie;
    ImageDemoThreadControler controler;
};

//ImageSupporter *ImageSupporter::m_instance = nullptr;

#endif // MAINWINDOW_H
