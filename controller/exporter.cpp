#include "exporter.h"
#include "controller/databasemanager.h"
#include "utils/imageutils.h"

#include <QFileDialog>
#include <QDir>
#include <QStandardPaths>
#include <QDebug>

Exporter *Exporter::m_exporter = NULL;
Exporter *Exporter::instance()
{
    if (!m_exporter) {
        m_exporter = new Exporter();
    }

    return m_exporter;
}


Exporter::Exporter(QObject *parent)
    : QObject(parent)
{
}

void Exporter::exportImage(const QStringList imagePaths) {
    if (imagePaths.isEmpty()) {
        return;
    } else if (imagePaths.length() == 1) {
        QFileDialog exportDialog;
        //Todo: need to filter the format of images.
        QString imagePath = imagePaths.at(0);
        QString imageName = QString("%1.%2").arg(QFileInfo(imagePath).baseName())
                .arg(QFileInfo(imagePath).completeSuffix());
        QString picLocation = QStandardPaths::standardLocations(QStandardPaths::PicturesLocation).at(0);
        QString imageSavePath = QString("%1/%2").arg(picLocation).arg(imageName);
        qDebug() << "imageSavePath:" << imageSavePath;
        QString selectFilter = tr("JPEG(*.bmp *.gif *.jpg;*.png *.pbm;*.pgm *.ppm *.xbm *.xpm *.svg *.dds *.icns"
                                  "*.jp2 *.mng *.tga *.tiff *.wbmp *.webp;)");
        QString dialogFilePath = exportDialog.getSaveFileName(nullptr, tr("Save File"),
        imageSavePath, tr("All files (*.*);;JPEG (*.jpg *.jpeg);;""TIFF (*.tif);;PNG(*.png);;PBM(*.pbm *.pgm *.ppm *xbm,"
                          "*.xpm);; SVG(*.svg);;DDS(*.dds);;ICNS(*.icns);;JPG2(*.jp2);;MNG(*.mng);;TGA(*.tga);;WBMP(*wbmp);;WEBP(*.webp)"),
        &selectFilter, QFileDialog::DontUseNativeDialog);

        qDebug() << "dialogFilePath:" << dialogFilePath;
        QPixmap tmpImage(imagePaths.at(0));
        if (!tmpImage.isNull() && utils::image::imageIsSupport(dialogFilePath))
            tmpImage.save(dialogFilePath);
    } else {
        popupDialogSaveImage(imagePaths);
    }
}

void Exporter::exportAlbum(const QString &albumname) {
    QList<DatabaseManager::ImageInfo> albumImageInfos =
            DatabaseManager::instance()->getImageInfosByAlbum(albumname);
    QStringList tmpImagePaths = QStringList();
    for (int i = 0; i < albumImageInfos.length(); i++) {
        qDebug() << "album:" << albumImageInfos[i].path;
        tmpImagePaths.append(albumImageInfos[i].path);
    }

    popupDialogSaveImage(tmpImagePaths);
}

void Exporter::popupDialogSaveImage(const QStringList imagePaths) {
    QFileDialog exportDialog;
    QString exportdir = exportDialog.getExistingDirectory(nullptr, tr("Save File"),
           QStandardPaths::standardLocations(QStandardPaths::PicturesLocation).at(0),
                                                          QFileDialog::ShowDirsOnly);
    qDebug() << "popupDialogSaveImage:" << exportdir;
    for (int j(0); j < imagePaths.length(); j++) {

        if(utils::image::imageIsSupport(imagePaths[j])) {
            QPixmap tmpImage(imagePaths[j]);
            QString savePath =  QString("%1/%2.%3").arg(exportdir).arg(QFileInfo(imagePaths[j])
        .baseName()).arg(QFileInfo(imagePaths[j]).completeSuffix());
            if (!tmpImage.isNull() && utils::image::imageIsSupport(savePath))
            tmpImage.save(savePath);
        } else {
            continue;
        }
    }
}
