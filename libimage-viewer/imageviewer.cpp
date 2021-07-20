#include "imageviewer.h"

#include <QDebug>
ImageViewer::ImageViewer(QWidget *parent)
    : DWidget(parent)
{
    qDebug() << "test";
}

ImageViewer::~ImageViewer()
{

}
