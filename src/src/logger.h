// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef IMAGEVIEWER_LOGGER_H
#define IMAGEVIEWER_LOGGER_H

#include <QObject>
#include <dtkcore_global.h>

#ifdef DTKCORE_CLASS_DConfigFile
DCORE_BEGIN_NAMESPACE
class DConfig;
DCORE_END_NAMESPACE
#endif

// 动态日志规则监听：从 DConfig(org.deepin.dde.imageviewer) 读取 log_rules，
// 运行中随 debug-config 调试模式切换动态生效（参考 deepin-reader 的 MLogger）
class MLogger : public QObject
{
    Q_OBJECT
public:
    explicit MLogger(QObject *parent = nullptr);
    ~MLogger();

    inline QString rules() const { return m_rules; }
    void setRules(const QString &rules);

private:
    void appendRules(const QString &rules);

private:
    QString m_rules;
#ifdef DTKCORE_CLASS_DConfigFile
    Dtk::Core::DConfig *m_config;
#endif
};

#endif  // IMAGEVIEWER_LOGGER_H
