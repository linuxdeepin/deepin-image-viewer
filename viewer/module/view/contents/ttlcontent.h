#ifndef TTLCONTENT_H
#define TTLCONTENT_H

#include <QWidget>
#include <QLabel>
#include "controller/viewerthememanager.h"

class TTLContent : public QWidget
{
    Q_OBJECT
public:
    explicit TTLContent(bool inDB, QWidget *parent = 0);

signals:
    void clicked();
public slots:
    void setCurrentDir(QString text);
private slots:
    void onThemeChanged(ViewerThemeManager::AppTheme theme);
private:
    QLabel* m_curDirLabel;
};

#endif // TTLCONTENT_H
