#include "printhelper.h"
#include "printoptionspage.h"
#include "snifferimageformat.h"
#include <QPrintDialog>
#include <QPrintPreviewDialog>
#include <QPrintPreviewWidget>
#include <QPrinter>
#include <QPainter>
#include <QToolBar>
#include <QCoreApplication>
#include <QImageReader>
#include <QDebug>
//#if (DTK_VERSION >= DTK_VERSION_CHECK(5, 2, 2, 5))
#include <dprintpreviewwidget.h>
#include <dprintpreviewdialog.h>
//#endif

#ifdef USE_UNIONIMAGE
#include "unionimage.h"
#endif

PrintHelper::PrintHelper(QObject *parent)
    : QObject(parent)
{

}
//暂时没有使用配置文件的快捷键，现在是根据代码中的快捷键
/*
static QAction *hookToolBarActionIcons(QToolBar *bar, QAction **pageSetupAction = nullptr)
{
    QAction *last_action = nullptr;

    for (QAction *action : bar->actions()) {
        const QString &text = action->text();

        if (text.isEmpty())
            continue;

        // 防止被lupdate扫描出来
        const char *context = "QPrintPreviewDialog";
        const char *print = "Print";

        const QMap<QString, QString> map {
            {QCoreApplication::translate(context, "Next page"), QStringLiteral("go-next")},
            {QCoreApplication::translate(context, "Previous page"), QStringLiteral("go-previous")},
            {QCoreApplication::translate(context, "First page"), QStringLiteral("go-first")},
            {QCoreApplication::translate(context, "Last page"), QStringLiteral("go-last")},
            {QCoreApplication::translate(context, "Fit width"), QStringLiteral("fit-width")},
            {QCoreApplication::translate(context, "Fit page"), QStringLiteral("fit-page")},
            {QCoreApplication::translate(context, "Zoom in"), QStringLiteral("zoom-in")},
            {QCoreApplication::translate(context, "Zoom out"), QStringLiteral("zoom-out")},
            {QCoreApplication::translate(context, "Portrait"), QStringLiteral("layout-portrait")},
            {QCoreApplication::translate(context, "Landscape"), QStringLiteral("layout-landscape")},
            {QCoreApplication::translate(context, "Show single page"), QStringLiteral("view-page-one")},
            {QCoreApplication::translate(context, "Show facing pages"), QStringLiteral("view-page-sided")},
            {QCoreApplication::translate(context, "Show overview of all pages"), QStringLiteral("view-page-multi")},
            {QCoreApplication::translate(context, print), QStringLiteral("print")},
            {QCoreApplication::translate(context, "Page setup"), QStringLiteral("page-setup")}
        };


        const QString &icon_name = map.value(action->text());

        if (icon_name.isEmpty())
            continue;

        if (pageSetupAction && icon_name == "page-setup") {
            *pageSetupAction = action;
        }

        QIcon icon(QStringLiteral(":/qt-project.org/dialogs/assets/images/qprintpreviewdialog/images/%1-24.svg").arg(icon_name));
//        action->setIcon(icon);
        last_action = action;
    }

    return last_action;
}
*/
DDialog* PrintHelper::showPrintDialog(const QStringList &paths, QWidget *parent)
{
    QList<QImage> imgs;
    QImage img;
    for (const QString &path : paths) {
        QString errMsg;
        UnionImage_NameSpace::loadStaticImageFromFile(path, img, errMsg);
        if (!img.isNull()) {
            imgs << img;
        }
    }
    DPrintPreviewDialog *printDialog2 =new DPrintPreviewDialog(parent);
    QObject::connect(printDialog2, &DPrintPreviewDialog::paintRequested, parent, [ = ](DPrinter * _printer) {
        QPainter painter(_printer);
        for (QImage img : imgs) {
            if (!img.isNull()) {
                painter.setRenderHint(QPainter::Antialiasing);
                painter.setRenderHint(QPainter::SmoothPixmapTransform);
                QRect wRect  = _printer->pageRect();
                QImage tmpMap;
                if (img.width() > wRect.width() || img.height() > wRect.height()) {
                    tmpMap = img.scaled(wRect.size(), Qt::KeepAspectRatio, Qt::SmoothTransformation);
                } else {
                    tmpMap = img;
                }
                QRectF drawRectF = QRectF(qreal(wRect.width() - tmpMap.width()) / 2,
                                          qreal(wRect.height() - tmpMap.height()) / 2,
                                          tmpMap.width(), tmpMap.height());
                painter.drawImage(QRectF(drawRectF.x(), drawRectF.y(), tmpMap.width(),
                                         tmpMap.height()), tmpMap);
            }
            if (img != imgs.last()) {
                _printer->newPage();
                qDebug() << "painter newPage!    File:" << __FILE__ << "    Line:" << __LINE__;
            }
        }
        painter.end();
    });
//    printDialog2->exec();
    return static_cast<DDialog *>(printDialog2);

}

//QSize PrintHelper::adjustSize(PrintOptionsPage *optionsPage, QImage img, int resolution, const QSize &viewportSize)
//{
//    PrintOptionsPage::ScaleMode scaleMode = optionsPage->scaleMode();
//    QSize size(img.size());

//    if (scaleMode == PrintOptionsPage::ScaleToPage) {
//        size.scale(viewportSize, Qt::KeepAspectRatio);
//    } else if (scaleMode == PrintOptionsPage::ScaleToExpanding) {
//        size.scale(viewportSize, Qt::KeepAspectRatioByExpanding);
//    } else if (scaleMode == PrintOptionsPage::ScaleToCustomSize) {
//        double imageWidth = optionsPage->scaleWidth();
//        double imageHeight = optionsPage->scaleHeight();
//        size.setWidth(int(imageWidth * resolution));
//        size.setHeight(int(imageHeight * resolution));
//    } else {
//        const double inchesPerMeter = 100.0 / 2.54;
//        int dpmX = img.dotsPerMeterX();
//        int dpmY = img.dotsPerMeterY();
//        if (dpmX > 0 && dpmY > 0) {
//            double wImg = double(size.width()) / double(dpmX) * inchesPerMeter;
//            double hImg = double(size.height()) / double(dpmY) * inchesPerMeter;
//            size.setWidth(int(wImg * resolution));
//            size.setHeight(int(hImg * resolution));
//        }
//    }
//    qDebug() << "adjustSize:" << size <<"    File:" << __FILE__ << "    Line:" << __LINE__;
//    return size;
//}

//QPoint PrintHelper::adjustPosition(PrintOptionsPage *optionsPage, const QSize &imageSize, const QSize &viewportSize)
//{
//    Qt::Alignment alignment = optionsPage->alignment();
//    int posX, posY;

//    if (alignment & Qt::AlignLeft) {
//        posX = 0;
//    } else if (alignment & Qt::AlignHCenter) {
//        posX = (viewportSize.width() - imageSize.width()) / 2;
//    } else {
//        posX = viewportSize.width() - imageSize.width();
//    }

//    if (alignment & Qt::AlignTop) {
//        posY = 0;
//    } else if (alignment & Qt::AlignVCenter) {
//        posY = (viewportSize.height() - imageSize.height()) / 2;
//    } else {
//        posY = viewportSize.height() - imageSize.height();
//    }
//    qDebug() << "adjustPosition X:" << posX << "Y:" << posY <<"    File:" << __FILE__ << "    Line:" << __LINE__;
//    return QPoint(posX, posY);
//}
