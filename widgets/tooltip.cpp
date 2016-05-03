#include "tooltip.h"
#include <QFile>
#include <QDebug>

Tooltip::Tooltip(QWidget *parent) : QLabel(parent)
{
    initStyleSheet();
}

void Tooltip::initStyleSheet()
{
    QFile sf(":/qss/resources/qss/Tooltip.qss");
    if (!sf.open(QIODevice::ReadOnly)) {
        qWarning() << "Open style-sheet file error:" << sf.errorString();
        return;
    }

    this->setStyleSheet(QString(sf.readAll()));
    sf.close();
}
