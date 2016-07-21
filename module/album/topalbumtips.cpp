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

    int textHeight = utils::base::stringHeight(m_importButton->font(), m_importButton->text());
    m_importButton->setMinimumHeight(textHeight);

    connect(m_importButton, &QPushButton::clicked, this, [=] {
        emit SignalManager::instance()->addImageFromTimeline(m_album);
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

    m_infoLabel->setText(QString("%1").arg(trName(m_album)) + "  " + l);
}

void TopAlbumTips::setLeftMargin(int v)
{
    m_layout->setContentsMargins(v + 18, 0, 13, 0);
}

const QString TopAlbumTips::trName(const QString &name) const
{
    if (name == "My favorites") {
        return tr("My favorites");
    }
    else if (name == "Recent imported") {
        return tr("Recent imported");
    }
    else {
        return name;
    }
}
