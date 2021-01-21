/*
 * Copyright (C) 2016 ~ 2018 Deepin Technology Co., Ltd.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#ifndef COMMANDLINE_H
#define COMMANDLINE_H

#include <QObject>
#include <QCommandLineParser>

class MainWindow;
struct CMOption;
class CommandLine : public QObject
{
    Q_OBJECT
public:
    static CommandLine *instance();
    bool processOption();
    bool processOption(QDateTime time, bool newflag);
    ~CommandLine();
    MainWindow *getMainWindow();

private:
    void addOption(const CMOption *option);
    void showHelp();
    void viewImage(const QString& path, const QStringList& paths);

    explicit CommandLine();
    QString createOpenImageInfo(QString path, QStringList pathlist, QDateTime stime);
    void paraOpenImageInfo(QString source, QString &path, QStringList &pathlist);
private:
    static CommandLine *m_commandLine;
    QCommandLineParser m_cmdParser;
    MainWindow *m_mainWindow{nullptr};
};

#endif // COMMANDLINE_H
