#ifndef PRINTHELPER_H
#define PRINTHELPER_H

#include <QObject>
#include "printoptionspage.h"
#include <dprintpreviewwidget.h>
#include <dprintpreviewdialog.h>
//重构printhelper，因为dtk更新
//绘制图片处理类
class RequestedSlot : public QObject
{
    Q_OBJECT
public:
    explicit RequestedSlot(QObject *parent = nullptr);
    ~RequestedSlot();
public slots:
    void paintRequestedAsyn(DPrinter *_printer, const QVector<int> &pageRange);
    void paintRequestSync(DPrinter *_printer);

public:
    QStringList m_paths;
    QList<QImage> m_imgs;
};
class PrintHelper : public QObject
{
    Q_OBJECT

public:
    static PrintHelper *getIntance();
    explicit PrintHelper(QObject *parent = nullptr);

    //    static QSize adjustSize(PrintOptionsPage* optionsPage, QImage img, int resolution, const QSize & viewportSize);
    //    static QPoint adjustPosition(PrintOptionsPage* optionsPage, const QSize& imageSize, const QSize & viewportSize);
    void showPrintDialog(const QStringList &paths, QWidget *parent= nullptr);

    RequestedSlot *m_re = nullptr;
private:
    static PrintHelper *m_Printer;
};

#endif // PRINTHELPER_H
