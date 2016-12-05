#ifndef CONTENTSFRAME_H
#define CONTENTSFRAME_H

#include "titlebutton.h"
#include <QWidget>
#include <QScrollArea>

class QVBoxLayout;
class ContentsFrame : public QWidget
{
    Q_OBJECT
public:
    explicit ContentsFrame(QWidget *parent);
    void setCurrentID(const TitleButton::SettingID id);

signals:
    void currentFieldChanged(TitleButton::SettingID id);

private:
    void initScrollArea();

private:
    QScrollArea *m_area;
    QVBoxLayout *m_layout;
};

#endif // CONTENTSFRAME_H
