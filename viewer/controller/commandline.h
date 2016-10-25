#ifndef COMMANDLINE_H
#define COMMANDLINE_H

#include <QObject>
#include <QCommandLineParser>


struct CMOption;
class CommandLine : public QObject {
    Q_OBJECT
public:
    static CommandLine *instance();
    bool processOption();
    ~CommandLine();

private:
    void addOption(const CMOption *option);
    void showHelp();
    void viewImage(const QString &path, const QStringList &paths);

    explicit CommandLine();

private:
    static CommandLine *m_commandLine;
    QCommandLineParser m_cmdParser;
};

#endif // COMMANDLINE_H
