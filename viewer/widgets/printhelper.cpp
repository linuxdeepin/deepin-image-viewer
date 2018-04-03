#include "printhelper.h"
#include <QDebug>

PrintHelper::PrintHelper(QObject *parent)
    : QObject(parent)
{

}

QSize PrintHelper::adjustSize(PrintOptionsPage* optionsPage, QImage img, int resolution, const QSize & viewportSize)
{
    PrintOptionsPage::ScaleMode scaleMode = optionsPage->scaleMode();
    QSize size(img.size());

    if (scaleMode == PrintOptionsPage::ScaleToPage) {
        size.scale(viewportSize, Qt::KeepAspectRatio);
    } else if (scaleMode == PrintOptionsPage::ScaleToExpanding) {
        size.scale(viewportSize, Qt::KeepAspectRatioByExpanding);
    } else if (scaleMode == PrintOptionsPage::ScaleToCustomSize) {
        double imageWidth = optionsPage->scaleWidth();
        double imageHeight = optionsPage->scaleHeight();
        size.setWidth(int(imageWidth * resolution));
        size.setHeight(int(imageHeight * resolution));
    } else {
        const double inchesPerMeter = 100.0 / 2.54;
        int dpmX = img.dotsPerMeterX();
        int dpmY = img.dotsPerMeterY();
        if (dpmX > 0 && dpmY > 0) {
            double wImg = double(size.width()) / double(dpmX) * inchesPerMeter;
            double hImg = double(size.height()) / double(dpmY) * inchesPerMeter;
            size.setWidth(int(wImg * resolution));
            size.setHeight(int(hImg * resolution));
        }
    }

    return size;
}

QPoint PrintHelper::adjustPosition(PrintOptionsPage* optionsPage, const QSize& imageSize, const QSize & viewportSize)
{
    Qt::Alignment alignment = optionsPage->alignment();
    int posX, posY;

    if (alignment & Qt::AlignLeft) {
        posX = 0;
    } else if (alignment & Qt::AlignHCenter) {
        posX = (viewportSize.width() - imageSize.width()) / 2;
    } else {
        posX = viewportSize.width() - imageSize.width();
    }

    if (alignment & Qt::AlignTop) {
        posY = 0;
    } else if (alignment & Qt::AlignVCenter) {
        posY = (viewportSize.height() - imageSize.height()) / 2;
    } else {
        posY = viewportSize.height() - imageSize.height();
    }

    return QPoint(posX, posY);
}
