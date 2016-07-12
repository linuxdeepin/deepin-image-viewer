#ifndef PROGRESSWIDGETSTIPS_H
#define PROGRESSWIDGETSTIPS_H

#include <QDialog>
#include <QPushButton>
#include <QResizeEvent>
#include <dcircleprogress.h>
#include <dimagebutton.h>

using namespace Dtk::Widget;

class ProgressWidgetsTips : public QWidget {
    Q_OBJECT
public:
    ProgressWidgetsTips(QWidget* parent = 0);
    ~ProgressWidgetsTips();
    //set progress 's value
public slots:
    void setValue(int value);
    void setTitle(QString title);
    void setTips(QString tips);

signals:
    void progressValueChanged(int value);
    void stopProgress();
    void finished();
protected:

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

#endif // PROGRESSWIDGETSTIPS_H
