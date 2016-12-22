#ifndef IMPORTFRAME_H
#define IMPORTFRAME_H

#include <QWidget>

#include "controller/viewerthememanager.h"

class QPushButton;
class QLabel;
class ImportFrame : public QWidget
{
    Q_OBJECT
public:
    explicit ImportFrame(QWidget *parent = 0);
    void setTitle(const QString &title);
    void setButtonText(const QString &text);
    const QString buttonText() const;
private slots:
    void onThemeChanged(ViewerThemeManager::AppTheme theme);
signals:
    void clicked();
private:
    QPushButton *m_importButton;
    QLabel *m_bgLabel;
    QLabel *m_titleLabel;
};

#endif // IMPORTFRAME_H
