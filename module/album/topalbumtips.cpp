#include "topalbumtips.h"
#include "controller/databasemanager.h"
#include "controller/signalmanager.h"
#include <QLabel>
#include <QPushButton>
#include <QHBoxLayout>
#include <QDebug>

TopAlbumTips::TopAlbumTips(QWidget *parent) : QFrame(parent)
{
    setFixedHeight(24);

    m_infoLabel = new QLabel();
    m_infoLabel->setObjectName("AlbumInfoTipsLabel");

    m_importButton = new QPushButton(tr("Add image"));
    m_importButton->setObjectName("ImportFromTimelineButton");
    connect(m_importButton, &QPushButton::clicked, this, [=] {
        emit SignalManager::instance()->selectImageFromTimeline(m_album);
        qDebug() << "Importing images to album: " << m_album;
    });

    QHBoxLayout *layout = new QHBoxLayout(this);
    layout->setContentsMargins(13, 0, 13, 0);
    layout->addWidget(m_infoLabel);
    layout->addStretch(1);
    layout->addWidget(m_importButton);
}

void TopAlbumTips::setAlbum(const QString &album)
{
    m_album = album;
    DatabaseManager::AlbumInfo info
            = DatabaseManager::instance()->getAlbumInfo(album);
    const QString beginTime = info.earliestTime.toString(DATETIME_FORMAT);
    const QString endTime = info.latestTime.toString(DATETIME_FORMAT);

    m_infoLabel->setText(album + "  " + beginTime + "-" + endTime);
}
