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
#ifndef EXPORTER_H
#define EXPORTER_H

#include <QObject>
#include <QMap>

class Exporter : public QObject {
    Q_OBJECT
public:
    static Exporter *instance();

public slots:
    void exportImage(const QStringList imagePaths);
    void exportAlbum(const QString &albumname);
    void popupDialogSaveImage(const QStringList imagePaths);
private:
    explicit Exporter(QObject *parent = 0);
    static Exporter *m_exporter;
    QMap<QString, QString> m_picFormatMap;

    void initValidFormatMap();
    QString getOrderFormat(QString defaultFormat);
};

#endif // EXPORTER_H
