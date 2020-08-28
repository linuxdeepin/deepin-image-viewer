/*
 * Copyright (C) 2016 ~ 2018 Deepin Technology Co., Ltd.
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
#ifndef TTLCONTENT_H
#define TTLCONTENT_H

#include <QWidget>
#include <QLabel>
#include "controller/viewerthememanager.h"
#include "controller/dbmanager.h"
#include <DLabel>

DWIDGET_USE_NAMESPACE
typedef DLabel QLbtoDLabel;

class PushButton;
class ReturnButton;
class ElidedLabel;
class TTLContent : public QLbtoDLabel
{
    Q_OBJECT
public:
    explicit TTLContent(bool inDB, QWidget *parent = 0);

signals:
    void clicked();
    void resetTransform(bool fitWindow);
    void rotateClockwise();
    void rotateCounterClockwise();

    void removed();
    void imageEmpty(bool v);
    void contentWidthChanged(int width);

public slots:
    void setCurrentDir(QString text);
    void setImage(const QString &path,DBImgInfoList infos);
    void updateCollectButton();

private slots:
    void onThemeChanged(ViewerThemeManager::AppTheme theme);
    void updateFilenameLayout();

protected:
    void resizeEvent(QResizeEvent *event);

private:
#ifndef LITE_DIV
    PushButton *m_folderBtn;
    ReturnButton *m_returnBtn;
#endif
    bool m_inDB;

    PushButton* m_adaptImageBtn;
    PushButton* m_adaptScreenBtn;
    PushButton* m_clBT;
    PushButton* m_rotateLBtn;
    PushButton* m_rotateRBtn;
    PushButton* m_trashBtn;
    ElidedLabel* m_fileNameLabel;

    QString m_imagePath;
    int m_windowWidth;
    int m_contentWidth;
};

#endif // TTLCONTENT_H
