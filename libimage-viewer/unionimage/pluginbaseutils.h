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
#ifndef PLUGINBASEUTILS_H
#define PLUGINBASEUTILS_H

#include <QObject>
#include <QTimer>
#include <QColor>
#include <QMimeData>
#include <QFileInfoList>

namespace pluginUtils {

namespace base {
//void        copyOneImageToClipboard(const QString &path);
//void        copyImageToClipboard(const QStringList &paths);
//void        showInFileManager(const QString &path);
//int         stringHeight(const QFont &f, const QString &str);
//QString     hash(const QString &str);
//QString     SpliteText(const QString &text, const QFont &font, int nLabelSize);
//QDateTime   stringToDateTime(const QString &time);
//QString     getFileContent(const QString &file);
//QPixmap     renderSVG(const QString &filePath, const QSize &size);
bool checkMimeData(const QMimeData *mimeData);
QString mkMutiDir(const QString &path);

const QFileInfoList getImagesInfo(const QString &dir, bool recursive = true);
bool imageSupportRead(const QString &path);
//bool                                imageSupportSave(const QString &path);
QStringList supportedImageFormats();
}  // namespace base

}  // namespace utils

#endif // BASEUTILS_H
