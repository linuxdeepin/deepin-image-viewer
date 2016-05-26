#ifndef BLUREDIALOG_H
#define BLUREDIALOG_H

#include "blureframe.h"
#include <QMap>

class QHBoxLayout;
class QVBoxLayout;
class BlureDialog : public BlureFrame
{
    Q_OBJECT
public:
    explicit BlureDialog(QWidget *parent, QWidget *source);
    void addButton(const QString &name, int id);
    void setContent(QWidget *content);
    void close();

signals:
    void closed();
    void clicked(int id);

private:
    bool m_first;
    QVBoxLayout *m_contentLayout;
    QHBoxLayout *m_buttonsLayout;
};

#endif // BLUREDIALOG_H
