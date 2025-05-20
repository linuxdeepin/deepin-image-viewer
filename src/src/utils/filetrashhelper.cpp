// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "filetrashhelper.h"
#include "imagedata/imagefilewatcher.h"
#include "unionimage/baseutils.h"

#include <DSysInfo>

#include <QApplication>
#include <QDBusReply>
#include <QDebug>
#include <QFile>
#include <QFileSystemWatcher>
#include <QRegularExpression>
#include <QLoggingCategory>

Q_DECLARE_LOGGING_CATEGORY(logImageViewer)

DCORE_USE_NAMESPACE
/*!
   \brief 拷贝自 dde-file-manager 代码，看图中仅使用 kRemovable 访问可能被卸载的设备
 */
enum DeviceQueryOption {
    kNoCondition = 0,
    kMountable = 1,
    kMounted = 1 << 1,
    kRemovable = 1 << 2,
    kNotIgnored = 1 << 3,
    kNotMounted = 1 << 4,
    kOptical = 1 << 5,
    kSystem = 1 << 6,
    kLoop = 1 << 7,
};

/*!
   \brief Result type for moving a file to the Trash via DBus.
 */
enum DeletionResult {
    kTrashTimeout,   // default error
    kTrashInterfaceError,
    kTrashInvalidUrl,
    kTrashSuccess,
};

/*!
   \class FileTrashHelper::FileTrashHelper
   \brief 文件回收站辅助类
   \details 用于判断文件是否可被移动到回收站中，以及移动文件到回收站
   \note 强关联文管DBus接口，需注意对应接口更新
 */
FileTrashHelper::FileTrashHelper(QObject *parent)
    : QObject(parent)
{
    // DFM 设备管理接口，访问文件挂载信息
    if (DSysInfo::majorVersion() == "25") {
        m_dfmDeviceManager.reset(new QDBusInterface(QStringLiteral(V25_FILEMANAGER_DAEMON_SERVICE),
                                                    QStringLiteral(V25_FILEMANAGER_DAEMON_PATH),
                                                    QStringLiteral(V25_FILEMANAGER_DAEMON_INTERFACE)));
    } else {
        m_dfmDeviceManager.reset(new QDBusInterface(QStringLiteral(V23_FILEMANAGER_DAEMON_SERVICE),
                                                    QStringLiteral(V23_FILEMANAGER_DAEMON_PATH),
                                                    QStringLiteral(V23_FILEMANAGER_DAEMON_INTERFACE)));
    }

    qCInfo(logImageViewer) << "Device manager initialized - version:" << DSysInfo::majorVersion()
                           << "service:" << m_dfmDeviceManager.data()->service()
                           << "interface:" << m_dfmDeviceManager.data()->interface()
                           << "path:" << m_dfmDeviceManager.data()->path();
}

/*!
   \return 返回文件 \a url 是否可被移动到回收站中
 */
bool FileTrashHelper::fileCanTrash(const QUrl &url)
{
    qCDebug(logImageViewer) << "Checking if file can be trashed:" << url.toString();
    // 检测当前文件和上次是否相同
    QDir currentDir(url.path());
    if (lastDir != currentDir) {
        qCDebug(logImageViewer) << "Directory changed, resetting mount info";
        lastDir = currentDir;
        resetMountInfo();
    }

    queryMountInfo();

    if (isGvfsFile(url)) {
        qCDebug(logImageViewer) << "File is on GVFS, cannot be trashed";
        return false;
    }

    if (isExternalDevice(url.path())) {
        qCDebug(logImageViewer) << "File is on external device, cannot be trashed";
        return false;
    }

    qCDebug(logImageViewer) << "File can be trashed";
    return true;
}

/*!
   \return 移动文件 \a url 到回收站
 */
bool FileTrashHelper::moveFileToTrash(const QUrl &url)
{
    qCDebug(logImageViewer) << "Moving file to trash:" << url.toString();
    // 优先采用文管后端 DBus 服务
    int ret = moveFileToTrashWithDBus(url);

    if (kTrashInterfaceError == ret) {
        qCInfo(logImageViewer) << "DBus interface failed, falling back to v20 version";
        // rollback v20 interface
        if (Libutils::base::trashFile(url.path())) {
            qCDebug(logImageViewer) << "Successfully moved file to trash using v20 interface";
            return true;
        }
    }

    if (kTrashSuccess != ret) {
        qCWarning(logImageViewer) << "Failed to move file to trash:" << url.toString();
        return false;
    }

    qCDebug(logImageViewer) << "Successfully moved file to trash";
    return true;
}

/*!
   \return 返回删除文件 \a url 的结果
 */
bool FileTrashHelper::removeFile(const QUrl &url)
{
    qCDebug(logImageViewer) << "Removing file:" << url.toString();
    QFile file(url.path());
    bool ret = file.remove();
    if (!ret) {
        qCWarning(logImageViewer) << "Failed to remove file:" << url.toString() << "error:" << file.errorString();
    } else {
        qCDebug(logImageViewer) << "Successfully removed file";
    }
    return ret;
}

/*!
   \brief 重置文件挂载信息，仅在需要获取时重新取得挂载数据
    对于看图应用的特殊处理，在打开新的图片后，仅需在首次触发查询挂载设备信息时查询，
    对于中途取消挂载的设备，文件监控会提示文件不存在，因此无需处理;
    同时，看图只会访问同一目录内容，挂载信息仅需查询单次，无需重复查询或监控设备
 */
void FileTrashHelper::resetMountInfo()
{
    initData = false;
    mountDevices.clear();
}

/*!
   \brief 查询当前挂载设备信息
   \note 注意依赖文官接口的变动
 */
void FileTrashHelper::queryMountInfo()
{
    if (initData) {
        return;
    }

    qCDebug(logImageViewer) << "Querying mount information";
    initData = true;

    // 调用 DBus 接口查询可被卸载设备信息
    QDBusReply<QStringList> deviceListReply = m_dfmDeviceManager->call("GetBlockDevicesIdList", kRemovable);
    if (!deviceListReply.isValid()) {
        qCWarning(logImageViewer) << "Failed to get block devices list:" << deviceListReply.error().message();
        return;
    }

    qCDebug(logImageViewer) << "Found" << deviceListReply.value().size() << "removable devices";

    for (const QString &id : deviceListReply.value()) {
        QDBusReply<QVariantMap> deviceReply = m_dfmDeviceManager->call("QueryBlockDeviceInfo", id, false);
        if (!deviceReply.isValid()) {
            qCWarning(logImageViewer) << "Failed to query device info for" << id << ":" << deviceReply.error().message();
            continue;
        }

        const QVariantMap deviceInfo = deviceReply.value();
        if (QString("usb") == deviceInfo.value("ConnectionBus").toString()) {
            const QStringList mountPaths = deviceInfo.value("MountPoints").toStringList();
            if (mountPaths.isEmpty()) {
                const QString mountPath = deviceInfo.value("MountPoint").toString();
                if (!mountPath.isEmpty()) {
                    qCDebug(logImageViewer) << "Adding USB device mount point:" << mountPath;
                    mountDevices.insert(id, mountPath);
                }
            } else {
                for (const QString &mount : mountPaths) {
                    if (!mount.isEmpty()) {
                        qCDebug(logImageViewer) << "Adding USB device mount point:" << mount;
                        mountDevices.insert(id, mount);
                    }
                }
            }
        }
    }
}

/*!
   \return 返回文件路径 \a path 是否指向外部设备
 */
bool FileTrashHelper::isExternalDevice(const QString &path)
{
    qCDebug(logImageViewer) << "Checking if path is on external device:" << path;
    for (auto itr = mountDevices.begin(); itr != mountDevices.end(); ++itr) {
        if (path.startsWith(itr.value())) {
            qCDebug(logImageViewer) << "Path is on external device:" << itr.value();
            return true;
        }
    }

    qCDebug(logImageViewer) << "Path is not on external device";
    return false;
}

/*!
   \return 判断 \a url 是否为远程挂载路径，此路径下同样无法恢复文件
 */
bool FileTrashHelper::isGvfsFile(const QUrl &url) const
{
    if (!url.isValid()) {
        qCDebug(logImageViewer) << "Invalid URL for GVFS check";
        return false;
    }

    const QString &path = url.toLocalFile();
    static const QString gvfsMatch { "(^/run/user/\\d+/gvfs/|^/root/.gvfs/|^/media/[\\s\\S]*/smbmounts)" };
    QRegularExpression re { gvfsMatch };
    QRegularExpressionMatch match { re.match(path) };
    bool isGvfs = match.hasMatch();
    qCDebug(logImageViewer) << "GVFS check for path:" << path << "result:" << isGvfs;
    return isGvfs;
}

/*!
   \brief 通过后端文管接口将文件 \a url 移动到回收站
    注意文管接口是异步接口且没有返回值，此处通过文件变更信号判断文件是否被移动。
   \return 移动文件到回收站是否成功
 */
int FileTrashHelper::moveFileToTrashWithDBus(const QUrl &url)
{
    qCDebug(logImageViewer) << "Moving file to trash via DBus:" << url.toString();
    if (!url.isValid()) {
        qCWarning(logImageViewer) << "Invalid URL for trash operation";
        return kTrashInvalidUrl;
    }

    QStringList list;
    list << url.toString();
    // 优先采用文管后端的DBus服务
    QDBusInterface interface(QStringLiteral("org.freedesktop.FileManager1"),
                             QStringLiteral("/org/freedesktop/FileManager1"),
                             QStringLiteral("org.freedesktop.FileManager1"));

    QEventLoop loop;
    int waitRet = kTrashTimeout;
    // wait file deleted signal
    QString filePath = url.path();
    auto conn = QObject::connect(ImageFileWatcher::instance(),
                                 &ImageFileWatcher::imageFileChanged,
                                 this,
                                 [&filePath, &loop, &waitRet](const QString &imagePath) {
                                     // 删除信息未通过 DBus 返回，直接判断文件是否已被删除
                                     if ((imagePath == filePath) && !QFile::exists(filePath)) {
                                         qCDebug(logImageViewer) << "File successfully moved to trash:" << filePath;
                                         waitRet = kTrashSuccess;
                                         loop.quit();
                                     };
                                 });

    auto pendingCall = interface.asyncCall("Trash", list);
    // will return soon
    pendingCall.waitForFinished();

    if (pendingCall.isError()) {
        auto error = pendingCall.error();
        qCWarning(logImageViewer) << "DBus trash operation failed:" << error.name() << error.message();
        QObject::disconnect(conn);
        return kTrashInterfaceError;
    }

    if (kTrashSuccess != waitRet) {
        // FIX-273813 Wait up to 10 seconds
        static constexpr int kDelTimeout = 1000 * 10;
        QTimer::singleShot(kDelTimeout, &loop, &QEventLoop::quit);
        loop.exec();
    }

    QObject::disconnect(conn);
    if (waitRet == kTrashTimeout) {
        qCWarning(logImageViewer) << "Trash operation timed out";
    }
    return waitRet;
}
