#ifndef MAINWIDGET_H
#define MAINWIDGET_H

#include <QWidget>

class QLabel;
class MainWidget : public QWidget
{
    Q_OBJECT
public:
    explicit MainWidget(QWidget *parent = 0);
protected:
    void dragEnterEvent(QDragEnterEvent *e) Q_DECL_OVERRIDE;
    void dropEvent(QDropEvent *e) Q_DECL_OVERRIDE;

private:
    QLabel *m_pl;
};

#endif // MAINWIDGET_H
