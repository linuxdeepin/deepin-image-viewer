// SPDX-FileCopyrightText: 2023-2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "applicationadpator.h"

#include <QUrl>

ApplicationAdaptor::ApplicationAdaptor(FileControl *controller)
    : QDBusAbstractAdaptor(controller)
    , fileControl(controller)
{
}

/**
 * @brief 打开传入的图片文件
 * @param fileName 文件路径
 * @return 是否允许打开图片文件
 */
bool ApplicationAdaptor::openImageFile(const QString &fileName)
{
    if (fileControl) {
        const QUrl inputUrl(fileName);
        const QUrl normalizedUrl = inputUrl.isLocalFile()
                ? QUrl::fromLocalFile(inputUrl.toLocalFile())
                : inputUrl;
        const QString urlPath = normalizedUrl.toString();
        if (fileControl->isCanReadable(urlPath) && fileControl->isImage(urlPath)) {
            Q_EMIT fileControl->openImageFile(urlPath);
            return true;
        }
    }

    return false;
}
