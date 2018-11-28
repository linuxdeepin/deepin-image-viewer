#ifndef PRINTHELPER_H
#define PRINTHELPER_H

#include <QObject>
#include "printoptionspage.h"

class PrintHelper : public QObject
{
    Q_OBJECT

public:
    PrintHelper(QObject *parent = nullptr);

    static void showPrintDialog(const QStringList &paths, QWidget *parent = nullptr);

    static QSize adjustSize(PrintOptionsPage* optionsPage, QImage img, int resolution, const QSize & viewportSize);
    static QPoint adjustPosition(PrintOptionsPage* optionsPage, const QSize& imageSize, const QSize & viewportSize);
};

#endif // PRINTHELPER_H
