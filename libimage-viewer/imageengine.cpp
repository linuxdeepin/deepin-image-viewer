#include "imageengine.h"

#include <QDebug>
ImageEngine::ImageEngine(QWidget *parent)
    : QObject(parent)
{
    qDebug() << "test";
}

ImageEngine::~ImageEngine()
{

}
