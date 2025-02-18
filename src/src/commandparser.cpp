// SPDX-FileCopyrightText: 2025 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "commandparser.h"
#include "printdialog/printhelper.h"

#include <QUrl>
#include <QCoreApplication>
#include <QCommandLineOption>

CommandParser::CommandParser(QObject *parent)
{
    initialize();
}

CommandParser *CommandParser::instance()
{
    static CommandParser ins;
    return &ins;
}

bool CommandParser::isSet(const QString &name) const
{
    return cmdParser.isSet(name);
}

QString CommandParser::value(const QString &name) const
{
    return cmdParser.value(name);
}

void CommandParser::process()
{
    return process(qApp->arguments());
}

void CommandParser::process(const QStringList &arguments)
{
    QStringList args;
    for (const auto &arg : arguments) {
        if (!arg.startsWith("-")) {
            args.append(QUrl::fromPercentEncoding(arg.toStdString().c_str()));
        } else {
            args.append(arg);
        }
    }
    cmdParser.process(args);
}

void CommandParser::quickPrint()
{
    const QStringList &argumets = positionalArguments();
    if (argumets.isEmpty())
        return;

    PrintHelper::getIntance()->showPrintDialog(argumets);
}

void CommandParser::initialize()
{
    cmdParser.setApplicationDescription(QString("%1 helper").arg(QCoreApplication::applicationName()));
    initOptions();
    cmdParser.addHelpOption();
}

void CommandParser::initOptions()
{
    QCommandLineOption printOption("print");
    addOption(printOption);
}

void CommandParser::addOption(const QCommandLineOption &option)
{
    cmdParser.addOption(option);
}

QStringList CommandParser::positionalArguments() const
{
    return cmdParser.positionalArguments();
}
