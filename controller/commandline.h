#ifndef COMMANDLINE_H
#define COMMANDLINE_H

#include <QObject>
#include <QCommandLineParser>

struct CMOption;
class CommandLine : public QObject {
    Q_OBJECT
public:
    static CommandLine *instance();
    ~CommandLine();

private slots:
    void processOption();

private:
    void addOption(const CMOption *option);

    explicit CommandLine();

private:
    static CommandLine *m_commandLine;
    QCommandLineParser m_cmdParser;
};

#endif // COMMANDLINE_H
