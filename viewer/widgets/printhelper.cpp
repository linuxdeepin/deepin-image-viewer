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
#include <dprintpreviewwidget.h>
#include <dprintpreviewdialog.h>
#ifdef USE_UNIONIMAGE
#include "unionimage.h"
#endif

PrintHelper::PrintHelper(QObject *parent)
    : QObject(parent)
{

}

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

void PrintHelper::showPrintDialog(const QStringList &paths, QWidget *parent)
{
    //lmh20200901，全新用dtk的打印
    DPrintPreviewDialog printDialog2(nullptr);
    QObject::connect(&printDialog2,&DPrintPreviewDialog::paintRequested,parent,[=](DPrinter *_printer){
        QPainter painter(_printer);
        QList<QImage> imgs;
        QImage img;
#if USE_UNIONIMAGE
        for(const QString &path :paths){
            QString errMsg;
            UnionImage_NameSpace::loadStaticImageFromFile(path, img, errMsg);
            if(!img.isNull()){
                imgs<<img;
            }
        }
#else
        for(const QString &path :paths){
            QImage imgtmp(path);
            if(!imgtmp.isNull()){
                imgs<<imgtmp;
            }
        }
#endif
        int index=0;
        for(auto img:imgs){
            QPoint pos(0,0);
            painter.setWindow(img.rect());
            int x2=painter.window().right();
            int y2=painter.window().bottom();
            painter.drawImage(pos.x(),pos.y(),img,0,0,x2,y2);
            if(++index !=imgs.size()){
                _printer->newPage();
            }
        }
        painter.end();
    });
    printDialog2.exec();
}

QSize PrintHelper::adjustSize(PrintOptionsPage *optionsPage, QImage img, int resolution, const QSize &viewportSize)
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
    qDebug() << "adjustSize:" << size <<"    File:" << __FILE__ << "    Line:" << __LINE__;
    return size;
}

QPoint PrintHelper::adjustPosition(PrintOptionsPage *optionsPage, const QSize &imageSize, const QSize &viewportSize)
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
    qDebug() << "adjustPosition X:" << posX << "Y:" << posY <<"    File:" << __FILE__ << "    Line:" << __LINE__;
    return QPoint(posX, posY);
}
