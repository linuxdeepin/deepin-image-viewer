#ifndef COMMANDLINE_H
#define COMMANDLINE_H

#include <QObject>
#include <QCommandLineParser>
#include <QDBusAbstractAdaptor>

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


class DeepinImageViewerDBus: public QDBusAbstractAdaptor {
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "com.deepin.deepinimageviewer")

public:
    explicit DeepinImageViewerDBus(QObject *parent = nullptr);
    ~DeepinImageViewerDBus();

    Q_SLOT void backToMainWindow() const;
    Q_SLOT void enterAlbum(const QString &album);
    Q_SLOT void searchImage(const QString &keyWord);
    Q_SLOT void editImage(const QString &path);
};

#endif // COMMANDLINE_H
