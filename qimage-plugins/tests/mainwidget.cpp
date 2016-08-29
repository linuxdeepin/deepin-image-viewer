#include "mainwidget.h"
#include <QImage>
#include <QPainter>
#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>
#include <QFileDialog>
#include <QStandardPaths>
#include <QDebug>
#include <QDropEvent>
#include <QUrl>
#include <QMimeData>

MainWidget::MainWidget(QWidget *parent) : QWidget(parent)
{
    m_pl = new QLabel;
    m_pl->setPixmap(QPixmap("/home/wanqing-deepin/图片/formats/psd.psd"));

    QPushButton *btn = new QPushButton("Open Image");
    connect(btn, &QPushButton::clicked, this, [=] {
        const QString name =
                QFileDialog::getOpenFileName(this);
        m_pl->setPixmap(QPixmap(name));
    });

    QVBoxLayout *l = new QVBoxLayout(this);
    l->setContentsMargins(0, 0, 0, 0);
    l->addWidget(btn);
    l->addWidget(m_pl);

    this->resize(500, 500);
    this->setMaximumSize(1366, 768);

    setAcceptDrops(true);
}

void MainWidget::dragEnterEvent(QDragEnterEvent *e)
{
    e->accept();
}

void MainWidget::dropEvent(QDropEvent *e)
{
    QList<QUrl> urls = e->mimeData()->urls();
    if (urls.isEmpty()) {
        return;
    }

    QStringList paths;
    for (QUrl url : urls) {
        const QString path = url.toLocalFile();
        if (! QFileInfo(path).isDir()) {
            paths << path;
        }
    }
    if (! paths.isEmpty()) {
        qDebug() << "Load: " << paths.first();
        m_pl->setPixmap(QPixmap(paths.first()));
    }

    e->accept();
}

