// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef EXIFREADER_H
#define EXIFREADER_H

#include <QByteArray>
#include <QMap>
#include <QString>

namespace LibUnionImage_NameSpace {

// Adds EXIF fields found in the given image format to the metadata map.
void readExifMetadata(const QString &path, const QByteArray &format, QMap<QString, QString> &metadata);

// Returns the EXIF Orientation tag value (1-8) from the image, or 1 if absent.
int getExifOrientation(const QString &path);

} // namespace LibUnionImage_NameSpace

#endif // EXIFREADER_H
