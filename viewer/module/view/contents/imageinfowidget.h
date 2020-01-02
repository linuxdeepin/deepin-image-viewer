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
#ifndef IMAGEINFOWIDGET_H
#define IMAGEINFOWIDGET_H

#include "controller/viewerthememanager.h"
#include "widgets/themewidget.h"

#include <QWidget>
#include <QLabel>
#include <QScrollArea>
#include <QVector>
#include <DScrollArea>
#include <denhancedwidget.h>
#include <DDrawer>
DWIDGET_USE_NAMESPACE
//class DBaseExpand;
class QFormLayout;
class QVBoxLayout;
class ViewSeparator;
class ImageInfoWidget : public QFrame
{
    Q_OBJECT
public:
    explicit ImageInfoWidget(const QString &darkStyle,
                             const QString &lightStyle,
                             QWidget *parent = 0);
    void setImagePath(const QString &path);
    void updateInfo();
    int contentHeight() const;
//    QSize sizeHint() const override;
public slots:
    void onExpandChanged(const bool &e);

protected:
    void resizeEvent(QResizeEvent *e) Q_DECL_OVERRIDE;
    void timerEvent(QTimerEvent *e) Q_DECL_OVERRIDE;

private:
    void clearLayout(QLayout *layout);
    const QString trLabel(const char *str);
    void updateBaseInfo(const QMap<QString, QString> &infos);
    void updateDetailsInfo(const QMap<QString, QString> &infos);
    QList<DDrawer *> addExpandWidget(const QStringList &titleList);
    void initExpand(QVBoxLayout *layout, DDrawer *expand);

private:
    int m_updateTid = 0;
    int m_maxTitleWidth;  //For align colon
    int m_maxFieldWidth;
    bool m_isBaseInfo = false;
    bool m_isDetailsInfo = false;
    QString m_path;
    QFrame *m_exif_base = nullptr;
    QFrame *m_exif_details = nullptr;
    QFormLayout *m_exifLayout_base = nullptr;
    QFormLayout *m_exifLayout_details = nullptr;
    ViewSeparator *m_separator = nullptr;
    QList<DDrawer *> m_expandGroup;
    QVBoxLayout *m_mainLayout = nullptr;
    QScrollArea *m_scrollArea = nullptr;
    QString m_closedString;
};

#endif // IMAGEINFOWIDGET_H
