// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "imagefilewatcher.h"
#include "imageinfo.h"

#include <QDir>
#include <QFileInfo>
#include <QFileSystemWatcher>

ImageFileWatcher::ImageFileWatcher(QObject *parent)
    : QObject(parent)
    , fileWatcher(new QFileSystemWatcher(this))
{
    connect(fileWatcher, &QFileSystemWatcher::fileChanged, this, &ImageFileWatcher::onImageFileChanged);
    connect(fileWatcher, &QFileSystemWatcher::directoryChanged, this, &ImageFileWatcher::onImageDirChanged);
}

ImageFileWatcher::~ImageFileWatcher() {}

void ImageFileWatcher::resetImageFiles(const QStringList &filePaths)
{
    fileWatcher->removePaths(fileWatcher->files());
    fileWatcher->removePaths(fileWatcher->directories());

    for (const QString &filePath : filePaths) {
        QString tempPath = QUrl(filePath).toLocalFile();
        QFileInfo info(tempPath);
        // 若文件存在
        if (info.exists()) {
            // 记录文件的最后修改时间
            cacheFileInfo.insert(tempPath, filePath);
            // 将文件追加到记录中
            fileWatcher->addPath(tempPath);
        }
    }

    QStringList fileList = fileWatcher->files();
    if (!fileList.isEmpty()) {
        // 观察文件夹变更
        QFileInfo info(fileList.first());
        fileWatcher->addPath(info.absolutePath());
    }
}

void ImageFileWatcher::fileRename(const QString &oldPath, const QString &newPath)
{
    if (cacheFileInfo.contains(oldPath)) {
        fileWatcher->removePath(oldPath);

        cacheFileInfo.insert(newPath, newPath);
        fileWatcher->addPath(newPath);
    }
}

void ImageFileWatcher::onImageFileChanged(const QString &file)
{
    // 文件移动、删除或替换后触发
    if (cacheFileInfo.contains(file)) {
        QUrl url = cacheFileInfo.value(file);
        bool isExist = QFile::exists(file);
        // 文件移动或删除，缓存记录
        if (!isExist) {
            removedFile.insert(file, url);
        }

        Q_EMIT imageFileChanged(url);

        // 请求重新加载缓存，外部使用 ImageInfo 获取文件状态变更
        ImageInfo info;
        info.setSource(url);
        info.refresh();
    }
}

void ImageFileWatcher::onImageDirChanged(const QString &dir)
{
    // 文件夹变更，判断是否存在新增已移除的文件
    QDir imageDir(dir);
    QStringList dirFiles = imageDir.entryList();

    for (auto itr = removedFile.begin(); itr != removedFile.end();) {
        QFileInfo info(itr.key());
        if (dirFiles.contains(info.fileName())) {
            // 重新追加到文件观察中
            fileWatcher->addPath(itr.key());
            // 文件恢复或替换，发布文件变更信息
            onImageFileChanged(itr.key());

            // 从缓存信息中移除
            itr = removedFile.erase(itr);
        } else {
            ++itr;
        }
    }
}
