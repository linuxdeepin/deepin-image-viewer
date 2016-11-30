#ifndef TITLEFRAME_H
#define TITLEFRAME_H

#include "titlebutton.h"
#include <QWidget>

class TitleFrame : public QWidget
{
    Q_OBJECT
public:
    explicit TitleFrame(QWidget *parent = 0);
    void setCurrentID(TitleButton::SettingID id);

signals:
    void clicked(TitleButton::SettingID id);

protected:
    void paintEvent(QPaintEvent *e) Q_DECL_OVERRIDE;

private:
    void onButtonClicked(TitleButton::SettingID id);

private:
    QList<TitleButton *> m_buttonList;
};

#endif // TITLEFRAME_H
