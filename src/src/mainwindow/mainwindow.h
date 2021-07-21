/*
 * Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     ZhangYong <zhangyong@uniontech.com>
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
#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <DMainWindow>
#include <QListWidget>
#include <QListWidgetItem>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFrame>
#include <DTitlebar>
#include <QStackedWidget>
#include <DSearchEdit>
#include <DLabel>
#include <QStatusBar>
#include <DTabBar>
#include <QButtonGroup>
#include <DSuggestButton>
#include <DProgressBar>

DWIDGET_USE_NAMESPACE
class HomePageWidget;
class ImageViewer;
class MainWindow : public DMainWindow
{
    Q_OBJECT

public:
    //增加单例接口，方便closeFromMenu调用
    static MainWindow &instance()
    {
        static MainWindow w;
        return w;
    }

    ~MainWindow() override;
private:
    explicit MainWindow();
    void initUI();
protected:
    void wheelEvent(QWheelEvent *event) Q_DECL_OVERRIDE;
    void resizeEvent(QResizeEvent *e) Q_DECL_OVERRIDE;
    bool eventFilter(QObject *obj, QEvent *event) Q_DECL_OVERRIDE;
    void closeEvent(QCloseEvent *event) Q_DECL_OVERRIDE;
    void showEvent(QShowEvent *event) Q_DECL_OVERRIDE;
public slots:
    void slotOpenImg();
private:
    QStackedWidget *m_centerWidget = nullptr;
    HomePageWidget *m_homePageWidget = nullptr;
    ImageViewer *m_imageViewer = nullptr;
};

#endif // MAINWINDOW_H
