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

class QFormLayout;
class QVBoxLayout;
class ViewSeparator;
class ImageInfoWidget : public ThemeScrollArea
{
    Q_OBJECT
public:
    explicit ImageInfoWidget(const QString& darkStyle,
                             const QString& lightStyle,
                             QWidget *parent = 0);
    void setImagePath(const QString &path);
    void updateInfo();
//    QSize sizeHint() const override;

protected:
    void resizeEvent(QResizeEvent *e) Q_DECL_OVERRIDE;
    void timerEvent(QTimerEvent *e) Q_DECL_OVERRIDE;

private:
    void clearLayout(QLayout* layout);
    const QString trLabel(const char *str);
    void updateBaseInfo(const QMap<QString, QString> &infos);
    void updateDetailsInfo(const QMap<QString, QString> &infos);

private:
    int m_updateTid = 0;
    int m_maxTitleWidth;  //For align colon
    int m_maxFieldWidth;
    QString m_path;
    QFormLayout* m_exifLayout_base;
    QFormLayout* m_exifLayout_details;
    ViewSeparator* m_separator;
};

#endif // IMAGEINFOWIDGET_H
