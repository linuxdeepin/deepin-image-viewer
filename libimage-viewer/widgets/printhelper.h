/*
 * Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     LiuMingHang <liuminghang@uniontech.com>
 *
 * Maintainer: ZhangYong <ZhangYong@uniontech.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#ifndef PRINTHELPER_H
#define PRINTHELPER_H

#include <QObject>

#include <dprintpreviewwidget.h>
#include <dprintpreviewdialog.h>
DWIDGET_USE_NAMESPACE
//重构printhelper，因为dtk更新
//绘制图片处理类
class RequestedSlot : public QObject
{
    Q_OBJECT
public:
    explicit RequestedSlot(QObject *parent = nullptr);
    ~RequestedSlot();
private slots:
    void paintRequestSync(DPrinter *_printer);

public:
    QStringList m_paths;
    QList<QImage> m_imgs;
};
class PrintHelper : public QObject
{
    Q_OBJECT

public:
    static PrintHelper *getIntance();
    explicit PrintHelper(QObject *parent = nullptr);

    //    static QSize adjustSize(PrintOptionsPage* optionsPage, QImage img, int resolution, const QSize & viewportSize);
    //    static QPoint adjustPosition(PrintOptionsPage* optionsPage, const QSize& imageSize, const QSize & viewportSize);
    void showPrintDialog(const QStringList &paths, QWidget *parent = nullptr);

    RequestedSlot *m_re = nullptr;
private:
    static PrintHelper *m_Printer;
};

#endif // PRINTHELPER_H
