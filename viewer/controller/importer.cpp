#include "importer.h"
#include "application.h"
#include "controller/dbmanager.h"
#include "controller/signalmanager.h"
#include "utils/imageutils.h"
#include <QDebug>
#include <QDir>
#include <QDirIterator>
#include <QFileDialog>
#include <QFileInfo>

Importer::Importer(QObject *parent)
    : QObject(parent)
{

}

Importer *Importer::m_importer = NULL;
Importer *Importer::instance()
{
    if (!m_importer) {
        m_importer = new Importer();
    }

    return m_importer;
}

void Importer::showImportDialog(const QString &album)
{
    QString dir = QFileDialog::getExistingDirectory(
                nullptr, tr("Open Directory"), QDir::homePath(),
                QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);

    importDir(dir, album);
}

void Importer::importDir(const QString &path, const QString &album)
{
    const QFileInfoList fileInfos = utils::image::getImagesInfo(path);
    if( ! QDir(path).exists() || fileInfos.isEmpty() ) {
        emit imported(false);
        return;
    }

    // To avoid the repeat paths
    QStringList paths;
    DBImgInfoList imgInfos;
    for (QFileInfo finfo : fileInfos) {
        if (paths.contains(finfo.absoluteFilePath()))
            continue;
        QString path = finfo.absoluteFilePath().toUtf8().toPercentEncoding("/");
        paths << path;
        DBImgInfo imgInfo;
        imgInfo.fileName = finfo.fileName().toUtf8().toPercentEncoding();
        imgInfo.filePath = path;
        imgInfo.time = utils::image::getCreateDateTime(finfo.absoluteFilePath());

        imgInfos << imgInfo;
    }
    dApp->dbM->insertImgInfos(imgInfos);
    dApp->dbM->insertIntoAlbum(album, paths);


    emit imported(true);
}

void Importer::importFiles(const QStringList &paths, const QString &album)
{
    if (paths.isEmpty()) {
        emit imported(false);
        return;
    }
    DBImgInfoList imgInfos;
    QStringList ePaths;
    for (QString path : paths) {
        QFileInfo info(path);
        QString p = info.absoluteFilePath().toUtf8().toPercentEncoding("/");
        ePaths << p;
        DBImgInfo imgInfo;
        imgInfo.fileName = info.fileName().toUtf8().toPercentEncoding();
        imgInfo.filePath = p;
        imgInfo.time = utils::image::getCreateDateTime(path);
        imgInfos << imgInfo;

    }
    dApp->dbM->insertImgInfos(imgInfos);
    dApp->dbM->insertIntoAlbum(album, ePaths);

    emit imported(true);
}
