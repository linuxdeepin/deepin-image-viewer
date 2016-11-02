#include "popupdialogmanager.h"
#include "frame/deletedialog.h"
#include "dmenu.h"

#include <QtPrintSupport/QPrinter>
#include <QtPrintSupport/QPrintDialog>
#include <QPainter>
#include <QDebug>
#include <QImage>
#include <QPixmap>
#include <QFileInfo>
#include <QString>

using namespace Dtk::Widget;

namespace controller {
    namespace popup {
        bool printDialog(const QString imgPath) {
            QPrinter printer(QPrinter::ScreenResolution);
            printer.setOutputFormat(QPrinter::PdfFormat);
            QPixmap img;
            qDebug() << "img load result:" << img.load(imgPath);
            if (img.width() > img.height())
                printer.setPageOrientation(QPageLayout::Landscape);

            QRect pageOriginRect = printer.pageRect();
            QSize pageRect = QSize(pageOriginRect.width() - 8,
                                   pageOriginRect.height() - 8);

            img = img.scaled(pageRect, Qt::KeepAspectRatio, Qt::SmoothTransformation);

            QPrintDialog* printDialog = new QPrintDialog(&printer);
            printDialog->resize(400, 300);
            if (printDialog->exec() == QDialog::Accepted) {
                QPainter painter(&printer);
                painter.drawPixmap(0, 0, img);
                painter.end();
                qDebug() << "print succeed!";
                return true;
            }

            QObject::connect(printDialog, &QPrintDialog::finished, printDialog,
                    &QPrintDialog::deleteLater);

            qDebug() << "print failed!";
            return false;
        }  
    }
}
