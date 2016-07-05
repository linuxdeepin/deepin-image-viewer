#ifndef PROGRESSDIALOG_H
#define PROGRESSDIALOG_H

#include <QDialog>
#include <QMouseEvent>
#include <QPushButton>
#include <QResizeEvent>
#include <dcircleprogress.h>
#include <dimagebutton.h>

using namespace Dtk::Widget;

class ProgressDialog : public QDialog {
    Q_OBJECT
public:
    ProgressDialog(QWidget* parent = 0);
    ~ProgressDialog();
    //set progress 's value
public slots:
    void setValue(int value);
    void setTitle(QString title);
    void setTips(QString tips);
    void setPos(QPoint pos);
signals:
    void progressValueChanged(int value);
    void stopProgress();
    void finished();
protected:
    void mouseMoveEvent(QMouseEvent *event);
    void mousePressEvent(QMouseEvent *event);
    void mouseReleaseEvent(QMouseEvent *event);
    void resizeEvent(QResizeEvent* event);
private:
    void initUi();
    void initStyleSheet();
    void initConnect();

    DCircleProgress* m_cirProgress;
    DImageButton* m_cancelButton;
    QLabel* m_title;
    QLabel* m_tips;
    QPoint m_dragPos;
};

#endif // PROGRESSDIALOG_H
