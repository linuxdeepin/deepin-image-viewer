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
    const QString beginTime = utils::base::timeToString(info.beginTime, true);
    const QString endTime = utils::base::timeToString(info.endTime, true);

    if (m_album == "My favorites") {
        m_album = "我的收藏";
    } else if (m_album == "Recent imported") {
        m_album = "最近导入";
    }

    m_infoLabel->setText(QString("%1").arg(m_album) + "  " + beginTime + "-" + endTime);
}
