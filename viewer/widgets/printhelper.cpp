#include "printhelper.h"
#include "printoptionspage.h"
#include <QPrintDialog>
#include <QPrinter>
#include <QPainter>
#include <QDebug>

PrintHelper::PrintHelper(QObject *parent)
    : QObject(parent)
{

}

void PrintHelper::showPrintDialog(const QStringList &paths)
{
    QPrinter printer;
    QImage img;

    QPrintDialog* printDialog = new QPrintDialog(&printer);
    PrintOptionsPage *optionsPage = new PrintOptionsPage;
    printDialog->setOptionTabs(QList<QWidget *>() << optionsPage);
    printDialog->resize(400, 300);

    // HACK: Qt的打印设置有点bug，属性对话框中手动设置了纸张方向为横向（默认纵向）其实并不生效，
    //（猜测是透过cups协商出了问题，跟踪src/printsupport里面的代码没有问题，
    // 应该在src/plugins/printsupport中出的问题），
    // 如果在构造QPainter对象之前给QPrinter设置为横向，则实际可以横向打印，
    // 但是这时候手动选择纵向又不会生效。
    // 所以这里的hack是事先判断图像是“横向”还是“纵向”的，给QPrinter设置默认的纸张方向，
    // 以满足大部分图片打印的需求。
    QList<QImage> imgs;

    for (const QString &path : paths) {
        if (!img.load(path)) {
            qDebug() << "img load failed" << path;
            continue;
        }

        imgs << img;
    }

    if (!imgs.isEmpty())
    {
        QImage img1 = imgs.first();
        qDebug() << img1.width() << img1.height();
        if (!img1.isNull() && img1.width() > img1.height()) {
            printer.setPageOrientation(QPageLayout::Landscape);
        }
    }
    // HACK - end

    if (printDialog->exec() == QDialog::Accepted) {
        QPainter painter(&printer);

        for (const QImage img : imgs) {
            QRect rect = painter.viewport();
            QSize size = PrintHelper::adjustSize(optionsPage, img, printer.resolution(), rect.size());
            QPoint pos = PrintHelper::adjustPosition(optionsPage, size, rect.size());

            painter.setViewport(pos.x(), pos.y(), size.width(), size.height());
            painter.setWindow(img.rect());
            painter.drawImage(0, 0, img);

            if (img != imgs.last()) {
                printer.newPage();
            }
        }

        painter.end();
        qDebug() << "print succeed!";

        return;
    }

    QObject::connect(printDialog, &QPrintDialog::finished, printDialog,
                     &QPrintDialog::deleteLater);

    qDebug() << "print failed!";
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
