#ifndef SCANPATHSDIALOG_H
#define SCANPATHSDIALOG_H

#include <QFrame>
#include <DMainWindow>

DWIDGET_USE_NAMESPACE

class QLabel;
class QVBoxLayout;
class QStackedWidget;
class ScanPathsDialog : public DMainWindow
{
    Q_OBJECT
public:
    static ScanPathsDialog *instance();
    void addPath(const QString &path, bool check=true);
    void show();

signals:
    void requestUpdateCount();

protected:
    void timerEvent(QTimerEvent *e) Q_DECL_OVERRIDE;

private slots:
    void showSelectDialog();

private:
    explicit ScanPathsDialog(QWidget *parent = 0);
    void removePath(const QString &path);
    void initTitle();
    void initPathsArea();
    void initMessageLabel();
    void initAddButton();

    void showMessage(const QString &message);

    bool isLegalPath(const QString &path) const;
    bool isContainByScanPaths(const QString &path) const;
    bool isSubPathOfScanPaths(const QString &path) const;

    QStringList scanpaths() const;
    void addToScanPaths(const QString &path);
    void removeFromScanPaths(const QString &path);

private:
    static ScanPathsDialog *m_dialog;
    int m_messageTID;
    QLabel *m_messageLabel;
    QVBoxLayout *m_mainLayout;
    QVBoxLayout *m_pathsLayout;
    QStackedWidget *m_contentStack;
};

#endif // SCANPATHSDIALOG_H
