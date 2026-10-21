// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "logger.h"
#include <QLoggingCategory>
#include <QObject>

#ifdef DTKCORE_CLASS_DConfigFile
#include <DConfig>
#endif
DCORE_USE_NAMESPACE

MLogger::MLogger(QObject *parent)
    : QObject(parent), m_rules("")
#ifdef DTKCORE_CLASS_DConfigFile
    , m_config(nullptr)
#endif
{
    QByteArray logRules = qgetenv("QT_LOGGING_RULES");
    // qunsetenv 之前一定不要有任何日志打印，否则取消环境变量设置不会生效
    qunsetenv("QT_LOGGING_RULES");

    // set env
    m_rules = logRules;

#ifdef DTKCORE_CLASS_DConfigFile
    // set dconfig
    // DConfig 为运行时唯一权威来源：meta 未安装或创建失败时 create() 返回空指针，
    // 此时仅使用环境变量继承的规则兑底，不影响应用启动。
    m_config = DConfig::create("org.deepin.dde.imageviewer", "org.deepin.dde.imageviewer");
    if (m_config) {
        logRules = m_config->value("log_rules").toByteArray();
        appendRules(logRules);
        setRules(m_rules);

        // watch dconfig
        // 注：DConfig 变更后 setRules 会整体覆盖规则，即运行时以 DConfig 为唯一
        // 权威来源，环境变量 QT_LOGGING_RULES 仅作为 DConfig 不可用时的兑底（与
        // deepin-reader 的 MLogger 行为保持一致）。
        // LCOV_EXCL_START
        connect(m_config, &DConfig::valueChanged, this, [this](const QString &key) {
            if (key == "log_rules") {
                setRules(m_config->value(key).toByteArray());
            }
        });
        // LCOV_EXCL_STOP
    } else {
        setRules(m_rules);
    }
#else
    setRules(m_rules);
#endif
}

MLogger::~MLogger()
{
#ifdef DTKCORE_CLASS_DConfigFile
    if (m_config) {
        m_config->deleteLater();
    }
#endif
}

void MLogger::setRules(const QString &rules)
{
    auto tmpRules = rules;
    m_rules = tmpRules.replace(";", "\n");   // Qt 限制：setFilterRules 分隔符必须 \n 不能 ;
    QLoggingCategory::setFilterRules(m_rules);
}

void MLogger::appendRules(const QString &rules)
{
    QString tmpRules = rules;
    tmpRules = tmpRules.replace(";", "\n");
    // 按行精确去重，避免子串误判（如 "org...debug=false" 与 "*.debug=false"）
    const QStringList existing = m_rules.split('\n', Qt::SkipEmptyParts);
    QStringList adding = tmpRules.split('\n', Qt::SkipEmptyParts);
    for (int i = 0; i < adding.count(); ++i) {
        if (existing.contains(adding.at(i))) {
            adding.removeAt(i);
            --i;
        }
    }
    if (adding.isEmpty())
        return;
    m_rules.isEmpty() ? m_rules = adding.join("\n")
                      : m_rules += "\n" + adding.join("\n");
}
