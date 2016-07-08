#include "topalbumtips.h"
#include "controller/databasemanager.h"
#include "controller/signalmanager.h"
#include "utils/baseutils.h"
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

    m_layout = new QHBoxLayout(this);
    m_layout->setContentsMargins(13, 0, 13, 0);
    m_layout->addWidget(m_infoLabel);
    m_layout->addStretch(1);
    m_layout->addWidget(m_importButton);
}

void TopAlbumTips::setAlbum(const QString &album)
{
    m_album = album;
    DatabaseManager::AlbumInfo info
            = DatabaseManager::instance()->getAlbumInfo(album);
    const QString beginTime = utils::base::timeToString(info.beginTime, true);
    const QString endTime = utils::base::timeToString(info.endTime, true);
    const QString l = (beginTime.isEmpty() || endTime.isEmpty())
            ? "" : beginTime + "-" + endTime;

    if (m_album == "My favorites") {
        m_album = tr("My favorites");
    } else if (m_album == "Recent imported") {
        m_album = tr("Recent imported");
    }

    m_infoLabel->setText(QString("%1").arg(m_album) + "  " + l);
}

void TopAlbumTips::setLeftMargin(int v)
{
    m_layout->setContentsMargins(v + 18, 0, 13, 0);
}
