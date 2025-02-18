// SPDX-FileCopyrightText: 2025 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef COMMANDPARSER_H
#define COMMANDPARSER_H

#include <QCommandLineParser>

QT_BEGIN_NAMESPACE
class QCommandLineOption;
QT_END_NAMESPACE

class CommandParser : public QObject
{
    Q_OBJECT
public:
    static CommandParser *instance();

    bool isSet(const QString &name) const;
    QString value(const QString &name) const;
    void process();
    void process(const QStringList &arguments);

    void quickPrint();

private:
    explicit CommandParser(QObject *parent = nullptr);

    void initialize();
    void initOptions();
    void addOption(const QCommandLineOption &option);
    QStringList positionalArguments() const;

private:
    QCommandLineParser cmdParser;
};

#endif   // COMMANDPARSER_H
