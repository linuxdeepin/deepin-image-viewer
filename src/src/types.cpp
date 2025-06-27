// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "types.h"

#include <DLog>

Q_DECLARE_LOGGING_CATEGORY(logImageViewer)

Types::Types(QObject *parent)
    : QObject(parent)
{
    qCDebug(logImageViewer) << "Types constructor called";
}

Types::~Types()
{
    qCDebug(logImageViewer) << "Types destructor called";
}
