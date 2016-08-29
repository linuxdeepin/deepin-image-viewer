#include <QApplication>
#include <QLabel>
#include <QScopedPointer>
#include <QtDebug>
#include "FilterObj.h"
#include "FilterId.h"

int main(int argc, char** argv)
{
    QApplication a(argc, argv);
    QImage img(a.arguments().at(1));
    qDebug() << img;
    QScopedPointer<filter2d::FilterObj> f;
    f.reset(filter2d::FilterObj::create(filter2d::kSpread));
    f->setIndensity(1.0);
    f->setProperty("hue", 0.618);
    f->setProperty("contrast", 0.618);
    QLabel w, w0;
    w0.resize(img.width(), img.height());
    w0.show();
    //w0.move(w0.rect().topLeft() - QPoint(w0.width()/3, 0));
    w0.setPixmap(QPixmap::fromImage(img));
    w.resize(img.width(), img.height());
    w.show();
    w.move(w.rect().topLeft() + QPoint(w.width()*2/3, 0));
    w.setPixmap(QPixmap::fromImage(f->apply(img)));
    return a.exec();
}
