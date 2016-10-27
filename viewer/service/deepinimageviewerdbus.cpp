#include "application.h"
#include "controller/signalmanager.h"
#include "deepinimageviewerdbus.h"

#include <QDebug>

DeepinImageViewerDBus::DeepinImageViewerDBus(SignalManager *parent)
    : QDBusAbstractAdaptor(parent)
{

}

DeepinImageViewerDBus::~DeepinImageViewerDBus()
{

}

void DeepinImageViewerDBus::backToMainWindow() const
{
    emit parent()->backToMainPanel();
}

void DeepinImageViewerDBus::activeWindow() {
    emit parent()->activeWindow();
}

void DeepinImageViewerDBus::enterAlbum(const QString &album)
{
    qDebug() << "Enter the album: " << album;
    // TODO
}

void DeepinImageViewerDBus::searchImage(const QString &keyWord)
{
    qDebug() << "Go to search view and search image by: " << keyWord;
    // TODO
}

void DeepinImageViewerDBus::editImage(const QString &path)
{
    qDebug() << "Go to edit view and begin editing: " << path;
    emit parent()->editImage(path);
}
