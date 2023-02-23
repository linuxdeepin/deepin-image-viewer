// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef THUMBNAILCACHE_H
#define THUMBNAILCACHE_H

#include <QObject>
#include <QMutex>
#include <QHash>
#include <QImage>

class ThumbnailCache
{
    ThumbnailCache();
    ~ThumbnailCache();

public:
    static ThumbnailCache *instance();
    bool contains(const QString &path, int frameIndex = 0);
    QImage get(const QString &path, int frameIndex = 0);
    void add(const QString &path, int frameIndex, const QImage &image);
    void clear();

    static QString toFindKey(const QString &path, int frameIndex = 0);

private:
    QMutex mutex;
    QHash<QString, QImage> cache;
};

#endif // THUMBNAILCACHE_H
