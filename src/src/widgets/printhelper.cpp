#include "printhelper.h"

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

#ifdef USE_UNIONIMAGE
#include "printhelper.h"
#include "utils/unionimage.h"
#endif


PrintHelper *PrintHelper::m_Printer = nullptr;

PrintHelper *PrintHelper::getIntance()
{
    if (!m_Printer) {
        m_Printer = new PrintHelper();
    }
    return m_Printer;
}

PrintHelper::PrintHelper(QObject *parent)
    : QObject(parent)
{
    m_re = new RequestedSlot(this);
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
void PrintHelper::showPrintDialog(const QStringList &paths, QWidget *parent)
{
    Q_UNUSED(parent)
    m_re->m_paths.clear();
    m_re->m_imgs.clear();

    m_re->m_paths = paths;
    QImage imgTemp;
    for (const QString &path : paths) {
        QString errMsg;
        QImageReader imgReadreder(path);
        if(imgReadreder.imageCount()>1)
        {
            for(int imgindex=0;imgindex<imgReadreder.imageCount();imgindex++)
            {
                imgReadreder.jumpToImage(imgindex);
                m_re->m_imgs << imgReadreder.read();
            }
        }
        else {
            //QImage不应该多次赋值，所以换到这里来，修复style问题
            QImage img;
            UnionImage_NameSpace::loadStaticImageFromFile(path, img, errMsg);
            if (!img.isNull()) {
                m_re->m_imgs << img;
            }
        }


    }
    //适配打印接口2.0，dtk大于 5.4.4 版才合入最新的2.0打印控件接口
#if (DTK_VERSION_MAJOR > 5 \
    || (DTK_VERSION_MAJOR >=5 && DTK_VERSION_MINOR > 4) \
    || (DTK_VERSION_MAJOR >= 5 && DTK_VERSION_MINOR >= 4 && DTK_VERSION_PATCH > 4))//5.4.4暂时没有合入
    DPrintPreviewDialog printDialog2(nullptr);
    bool suc = printDialog2.setAsynPreview(m_re->m_imgs.size());//设置总页数，异步方式
    if (suc) {//异步
        connect(&printDialog2, SIGNAL(paintRequested(DPrinter *, const QVector<int> &)),
                m_re, SLOT(paintRequestedAsyn(DPrinter *, const QVector<int> &)));
    } else {//同步
        connect(&printDialog2, SIGNAL(paintRequested(DPrinter *)),
                m_re, SLOT(paintRequestSync(DPrinter *)));
    }
#else
    DPrintPreviewDialog printDialog2(nullptr);
    connect(&printDialog2, SIGNAL(paintRequested(DPrinter *)),
            m_re, SLOT(paintRequestSync(DPrinter *)));
#endif
#ifndef USE_TEST
    printDialog2.exec();
#else
    printDialog2.show();
#endif
    m_re->m_paths.clear();
    m_re->m_imgs.clear();
}

RequestedSlot::RequestedSlot(QObject *parent)
{
    Q_UNUSED(parent)
}

RequestedSlot::~RequestedSlot()
{

}
void RequestedSlot::paintRequestedAsyn(DPrinter *_printer, const QVector<int> &pageRange)
{
    QPainter painter(_printer);
    if (pageRange.size() > 0) {
        QImage img = m_imgs.at(pageRange.at(0) - 1);
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
    }
    painter.end();
}

void RequestedSlot::paintRequestSync(DPrinter *_printer)
{
    int currentIndex=0;
    QPainter painter(_printer);
    for (QImage img : m_imgs) {
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
        //不应该将多个相同的图片过滤掉
        if(currentIndex!=m_imgs.count()-1){
            _printer->newPage();
            currentIndex++;
        }
    }
    painter.end();
}

