#include "topalbumtips.h"
#include "application.h"
#include "controller/dbmanager.h"
#include "controller/signalmanager.h"
#include "utils/baseutils.h"
#include <QLabel>
#include <QPushButton>
#include <QHBoxLayout>
#include <QDebug>

TopAlbumTips::TopAlbumTips(QWidget *parent) : QFrame(parent)
{
    setFixedHeight(24);
    setAttribute(Qt::WA_TransparentForMouseEvents);
    m_infoLabel = new QLabel();
    m_infoLabel->setObjectName("AlbumInfoTipsLabel");

    QFont font;
    font.setWeight(25);
    m_infoLabel->setFont(font);
    m_layout = new QHBoxLayout(this);
    m_layout->setContentsMargins(13, 0, 13, 0);
    m_layout->addWidget(m_infoLabel);
    m_layout->addStretch(1);
}

void TopAlbumTips::setAlbum(const QString &album)
{
    auto info = dApp->dbM->getAlbumInfo(album);
    if (info.count == 0) {
        m_infoLabel->setText("");
    }
    else {
        const QString beginTime = info.beginTime.toString(tr("dd MMMM yyyy"));
        const QString endTime = info.endTime.toString(tr("dd MMMM yyyy"));
        const QString l = (beginTime.isEmpty() || endTime.isEmpty())
                ? "" : beginTime + "-" + endTime;

        m_infoLabel->setText(QString("%1").arg(trName(album)) + "  " + l);
    }
}

void TopAlbumTips::setLeftMargin(int v)
{
    m_layout->setContentsMargins(v + 18, 0, 13, 0);
}

const QString TopAlbumTips::trName(const QString &name) const
{
    if (name == "My favorite") {
        return tr("My favorite");
    }
    else if (name == "Recent imported") {
        return tr("Recent imported");
    }
    else {
        QFontMetrics fm(this->font());
        return fm.elidedText(name, Qt::ElideMiddle, 255);
    }
}
