#ifndef LOCKWIDGET_H
#define LOCKWIDGET_H

#include <QLabel>
#include "widgets/themewidget.h"

class LockWidget : public ThemeWidget {
    Q_OBJECT
public:
    LockWidget(const QString &darkFile, const QString &lightFile,
                  QWidget* parent = 0);
    ~LockWidget();

public slots:
    void setContentText(const QString &text);

private:
    QLabel *m_bgLabel;
    QLabel *m_lockTips;
};
#endif // LOCKWIDGET_H
