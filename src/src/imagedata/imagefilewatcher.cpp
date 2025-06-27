// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "imagefilewatcher.h"
#include "imageinfo.h"

#include <QDir>
#include <QFileInfo>
#include <QFileSystemWatcher>
#include <QLoggingCategory>

Q_DECLARE_LOGGING_CATEGORY(logImageViewer)

ImageFileWatcher::ImageFileWatcher(QObject *parent)
    : QObject(parent)
    , fileWatcher(new QFileSystemWatcher(this))
{
    qCDebug(logImageViewer) << "ImageFileWatcher constructor called.";
    connect(fileWatcher, &QFileSystemWatcher::fileChanged, this, &ImageFileWatcher::onImageFileChanged);
    connect(fileWatcher, &QFileSystemWatcher::directoryChanged, this, &ImageFileWatcher::onImageDirChanged);
    qCDebug(logImageViewer) << "Connected fileChanged and directoryChanged signals.";
}

ImageFileWatcher::~ImageFileWatcher() {
    qCDebug(logImageViewer) << "ImageFileWatcher destructor called.";
}

ImageFileWatcher *ImageFileWatcher::instance()
{
    qCDebug(logImageViewer) << "ImageFileWatcher::instance() called.";
    static ImageFileWatcher ins;
    return &ins;
}

/**
   @brief 重置监控文件列表为 \a filePaths , 若监控的文件路重复，则不执行重置
 */
void ImageFileWatcher::resetImageFiles(const QStringList &filePaths)
{
    qCDebug(logImageViewer) << "ImageFileWatcher::resetImageFiles() called with filePaths count: " << filePaths.count();
    // 重置时清理缓存记录
    cacheFileInfo.clear();
    qCDebug(logImageViewer) << "cacheFileInfo cleared.";
    removedFile.clear();
    qCDebug(logImageViewer) << "removedFile cleared.";
    rotateImagePathSet.clear();
    qCDebug(logImageViewer) << "rotateImagePathSet cleared.";

    if (filePaths.isEmpty()) {
        qCDebug(logImageViewer) << "File paths are empty, removing all current watchers.";
        auto files = fileWatcher->files();
        if (!files.isEmpty()) {
            fileWatcher->removePaths(fileWatcher->files());
            qCDebug(logImageViewer) << "Removed " << files.count() << " files from watcher.";
        }
        auto directories = fileWatcher->directories();
        if (!directories.isEmpty()) {
            fileWatcher->removePaths(fileWatcher->directories());
            qCDebug(logImageViewer) << "Removed " << directories.count() << " directories from watcher.";
        }
        qCDebug(logImageViewer) << "Cleared all file watchers.";
        return;
    }

    // 目前仅会处理单个文件夹路径，重复追加将忽略
    if (isCurrentDir(filePaths.first())) {
        qCDebug(logImageViewer) << "Directory already being watched:" << filePaths.first() << ", skipping reset.";
        return;
    }
    qCDebug(logImageViewer) << "Removing existing file and directory paths from watcher.";
    fileWatcher->removePaths(fileWatcher->files());
    fileWatcher->removePaths(fileWatcher->directories());

    for (const QString &filePath : filePaths) {
        QString tempPath = QUrl(filePath).toLocalFile();
        QFileInfo info(tempPath);
        // 若文件存在
        if (info.exists()) {
            qCDebug(logImageViewer) << "File exists, adding to watch:" << tempPath;
            // 记录文件的最后修改时间
            cacheFileInfo.insert(tempPath, filePath);
            // 将文件追加到记录中
            fileWatcher->addPath(tempPath);
            qCDebug(logImageViewer) << "Added file to watch:" << tempPath;
        } else {
            qCWarning(logImageViewer) << "File does not exist, not adding to watch:" << tempPath;
        }
    }

    QStringList fileList = fileWatcher->files();
    if (!fileList.isEmpty()) {
        // 观察文件夹变更
        QFileInfo info(fileList.first());
        QString dirPath = info.absolutePath();
        fileWatcher->addPath(dirPath);
        qCDebug(logImageViewer) << "Added directory to watch:" << dirPath;
    } else {
        qCDebug(logImageViewer) << "No files in watcher, skipping directory watch.";
    }
}

/**
   @brief 监控的文件 \a oldPath 重命名为 \a newPath 更新监控列表
 */
void ImageFileWatcher::fileRename(const QString &oldPath, const QString &newPath)
{
    qCDebug(logImageViewer) << "ImageFileWatcher::fileRename() called, oldPath: " << oldPath << ", newPath: " << newPath;
    if (cacheFileInfo.contains(oldPath)) {
        qCDebug(logImageViewer) << "Cached file info contains old path.";
        fileWatcher->removePath(oldPath);
        qCDebug(logImageViewer) << "Removed old path from watcher.";
        cacheFileInfo.insert(newPath, newPath);
        fileWatcher->addPath(newPath);
        qCInfo(logImageViewer) << "File renamed and watcher updated:" << oldPath << "->" << newPath;
    } else {
        qCDebug(logImageViewer) << "Cached file info does not contain old path: " << oldPath;
    }
}

/**
   @return 返回文件路径 \a filePath 所在的文件夹是否为当前监控的文件夹
 */
bool ImageFileWatcher::isCurrentDir(const QString &filePath)
{
    qCDebug(logImageViewer) << "ImageFileWatcher::isCurrentDir() called for filePath: " << filePath;
    QString dir = QFileInfo(filePath).absolutePath();
    qCDebug(logImageViewer) << "Extracted directory: " << dir;
    bool contains = fileWatcher->directories().contains(dir);
    qCDebug(logImageViewer) << "Watcher directories contain " << dir << ": " << contains;
    return contains;
}

/**
   @brief 设置当前旋转的图片文件路径为 \a targetPath ，在执行旋转操作前调用，
    旋转覆写文件时将不会发送变更信号(文件的旋转状态已在缓存中记录)
    文件旋转操作现移至子线程处理，和文件更新信号到达时间不定，因此在拷贝前记录操作文件，
    若旋转操作成功，通过文件更新处理重置；旋转操作失败，通过 cleareRotateStatus() 重置
 */
void ImageFileWatcher::recordRotateImage(const QString &targetPath)
{
    qCDebug(logImageViewer) << "ImageFileWatcher::recordRotateImage() called for targetPath: " << targetPath;
    rotateImagePathSet.insert(targetPath);
    qCDebug(logImageViewer) << "Added " << targetPath << " to rotate image path set.";
}

/**
   @brief 若 \a targetPath 和当前缓存的旋转记录文件一致，则清除记录。
 */
void ImageFileWatcher::clearRotateStatus(const QString &targetPath)
{
    qCDebug(logImageViewer) << "ImageFileWatcher::clearRotateStatus() called for targetPath: " << targetPath;
    if (rotateImagePathSet.contains(targetPath)) {
        rotateImagePathSet.remove(targetPath);
        qCDebug(logImageViewer) << "Removed " << targetPath << " from rotate image path set.";
    } else {
        qCDebug(logImageViewer) << "rotateImagePathSet does not contain " << targetPath << ", no action taken.";
    }
}

/**
   @brief 监控的文件路\a file 变更
 */
void ImageFileWatcher::onImageFileChanged(const QString &file)
{
    qCDebug(logImageViewer) << "ImageFileWatcher::onImageFileChanged() called for file: " << file;
    // 若为当前旋转处理的文件，不触发更新，状态在图像缓存中已同步
    if (rotateImagePathSet.contains(file)) {
        qCDebug(logImageViewer) << "Ignoring file change for rotating image:" << file;
        return;
    }

    // 文件移动、删除或替换后触发，旋转操作时不触发更新，使用缓存中的旋转图像
    if (cacheFileInfo.contains(file)) {
        QUrl url = cacheFileInfo.value(file);
        bool isExist = QFile::exists(file);
        qCDebug(logImageViewer) << "File exists status: " << isExist;
        // 文件移动或删除，缓存记录
        if (!isExist) {
            removedFile.insert(file, url);
            qCWarning(logImageViewer) << "File removed or moved, added to removedFile cache:" << file;
        } else {
            qCInfo(logImageViewer) << "File changed:" << file;
        }

        Q_EMIT imageFileChanged(file);
        qCDebug(logImageViewer) << "Emitted imageFileChanged signal for file: " << file;

        // 请求重新加载缓存，外部使用 ImageInfo 获取文件状态变更，使用 clearCurrentCache() 清理多页图缓存信息
        ImageInfo info;
        info.setSource(url);
        info.clearCurrentCache();
        info.reloadData();
        qCDebug(logImageViewer) << "ImageInfo cache cleared and reloaded for file: " << file;
    } else {
        qCDebug(logImageViewer) << "cacheFileInfo does not contain " << file << ", not processing change.";
    }
}

/**
   @brief 监控的文件路径 \a dir 变更
 */
void ImageFileWatcher::onImageDirChanged(const QString &dir)
{
    qCDebug(logImageViewer) << "ImageFileWatcher::onImageDirChanged() called for directory:" << dir;
    // 文件夹变更，判断是否存在新增已移除的文件
    QDir imageDir(dir);
    QStringList dirFiles = imageDir.entryList();
    qCDebug(logImageViewer) << "Files in directory: " << dirFiles.count();

    for (auto itr = removedFile.begin(); itr != removedFile.end();) {
        QFileInfo info(itr.key());
        if (dirFiles.contains(info.fileName())) {
            qCDebug(logImageViewer) << "Detected restored file: " << itr.key() << ", re-adding to watcher.";
            // 重新追加到文件观察中
            fileWatcher->addPath(itr.key());
            qCInfo(logImageViewer) << "File restored and re-added to watcher:" << itr.key();
            // 文件恢复或替换，发布文件变更信息
            onImageFileChanged(itr.key());

            // 从缓存信息中移除
            itr = removedFile.erase(itr);
            qCDebug(logImageViewer) << "Removed " << itr.key() << " from removedFile cache.";
        } else {
            qCDebug(logImageViewer) << "File " << itr.key() << " not found in directory, moving to next.";
            ++itr;
        }
    }
    qCDebug(logImageViewer) << "Directory change processing complete.";
}
