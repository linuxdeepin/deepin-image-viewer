// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef IMAGEFILEWATCHER_H
#define IMAGEFILEWATCHER_H

#include <QObject>
#include <QHash>

class QFileSystemWatcher;
class ImageFileWatcher : public QObject
{
    Q_OBJECT
public:
    explicit ImageFileWatcher(QObject *parent = nullptr);
    ~ImageFileWatcher() override;

    void resetImageFiles(const QStringList &filePaths);
    void fileRename(const QString &oldPath, const QString &newPath);

    Q_SIGNAL void imageFileChanged(const QUrl &imagePath);

private:
    // 当处理的图片文件被移动、替换、删除时触发
    Q_SLOT void onImageFileChanged(const QString &file);
    // 当处理的图片文件夹变更(新增图片等)
    Q_SLOT void onImageDirChanged(const QString &dir);

private:
    QHash<QString, QUrl> cacheFileInfo;  // 缓存的图片信息，用于判断图片信息是否变更 QHash<完整路径, url信息>
    QHash<QString, QUrl> removedFile;  // 缓存被移除的文件信息(FileWatcher在文件删除/移动后将不会继续观察)
    QFileSystemWatcher *fileWatcher = nullptr;  // 文件观察类，用于提示文件变更
};

#endif  // IMAGEFILEWATCHER_H
