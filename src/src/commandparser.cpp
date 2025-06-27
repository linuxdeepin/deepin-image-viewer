// SPDX-FileCopyrightText: 2025 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "commandparser.h"
#include "printdialog/printhelper.h"

#include <QUrl>
#include <QCoreApplication>
#include <QCommandLineOption>
#include <DLog>

Q_DECLARE_LOGGING_CATEGORY(logImageViewer)

CommandParser::CommandParser(QObject *parent)
{
    qCDebug(logImageViewer) << "CommandParser constructor called.";
    initialize();
    qCDebug(logImageViewer) << "CommandParser initialized.";
}

CommandParser *CommandParser::instance()
{
    qCDebug(logImageViewer) << "CommandParser::instance() called.";
    static CommandParser ins;
    return &ins;
}

bool CommandParser::isSet(const QString &name) const
{
    qCDebug(logImageViewer) << "CommandParser::isSet() called for option: " << name;
    return cmdParser.isSet(name);
}

QString CommandParser::value(const QString &name) const
{
    qCDebug(logImageViewer) << "CommandParser::value() called for option: " << name;
    return cmdParser.value(name);
}

void CommandParser::process()
{
    qCDebug(logImageViewer) << "CommandParser::process() called with default arguments.";
    return process(qApp->arguments());
}

void CommandParser::process(const QStringList &arguments)
{
    qCDebug(logImageViewer) << "CommandParser::process() called with arguments: " << arguments;
    QStringList args;
    for (const auto &arg : arguments) {
        qCDebug(logImageViewer) << "Processing argument: " << arg;
        if (!arg.startsWith("-")) {
            qCDebug(logImageViewer) << "Argument does not start with '-', decoding.";
            args.append(QUrl::fromPercentEncoding(arg.toStdString().c_str()));
        } else {
            args.append(arg);
        }
    }
    cmdParser.process(args);
    qCDebug(logImageViewer) << "CommandParser finished processing arguments.";
}

void CommandParser::quickPrint()
{
    qCDebug(logImageViewer) << "CommandParser::quickPrint() called.";
    const QStringList &argumets = positionalArguments();
    if (argumets.isEmpty()) {
        qCDebug(logImageViewer) << "No positional arguments for quick print, returning.";
        return;
    }

    PrintHelper::getIntance()->showPrintDialog(argumets);
    qCDebug(logImageViewer) << "Print dialog shown for arguments: " << argumets;
}

void CommandParser::initialize()
{
    qCDebug(logImageViewer) << "CommandParser::initialize() called.";
    cmdParser.setApplicationDescription(QString("%1 helper").arg(QCoreApplication::applicationName()));
    initOptions();
    qCDebug(logImageViewer) << "Initialized command line options.";
    cmdParser.addHelpOption();
    qCDebug(logImageViewer) << "Added help option.";
}

void CommandParser::initOptions()
{
    qCDebug(logImageViewer) << "CommandParser::initOptions() called.";
    QCommandLineOption printOption("print");
    addOption(printOption);
    qCDebug(logImageViewer) << "Added 'print' command line option.";
}

void CommandParser::addOption(const QCommandLineOption &option)
{
    qCDebug(logImageViewer) << "CommandParser::addOption() called for option: " << option.names();
    cmdParser.addOption(option);
}

QStringList CommandParser::positionalArguments() const
{
    qCDebug(logImageViewer) << "CommandParser::positionalArguments() called.";
    return cmdParser.positionalArguments();
}
