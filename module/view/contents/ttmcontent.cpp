#include "ttmcontent.h"
#include "controller/databasemanager.h"
#include "controller/signalmanager.h"
#include "widgets/imagebutton.h"
#include "utils/baseutils.h"
#include <QBoxLayout>
#include <QDebug>

namespace {

const QString FAVORITES_ALBUM = "My favorites";

}  // namespace

TTMContent::TTMContent(bool fromFileManager, QWidget *parent)
    : QWidget(parent),
      m_sManager(SignalManager::instance())
{
    QHBoxLayout *hb = new QHBoxLayout(this);
    hb->setContentsMargins(0, 0, 0, 0);
    hb->setSpacing(21);
    hb->addStretch();

    ImageButton *btn = new ImageButton();
    btn->setNormalPic(":/images/resources/images/adapt_image_normal.png");
    btn->setHoverPic(":/images/resources/images/adapt_image_hover.png");
    btn->setPressPic(":/images/resources/images/adapt_image_active.png");
    btn->setToolTip(tr("1:1 Size"));
    btn->setWhatsThis("1:1SizeButton");
    hb->addWidget(btn);
    connect(btn, &ImageButton::clicked, [this, btn](){
        if (btn->whatsThis() == "1:1SizeButton") {
            btn->setNormalPic(":/images/resources/images/adapt_screen_normal.png");
            btn->setHoverPic(":/images/resources/images/adapt_screen_hover.png");
            btn->setPressPic(":/images/resources/images/adapt_screen_active.png");
            btn->setToolTip(tr("Fit to window"));
            btn->setWhatsThis("FitToWindowButton");
            emit resetTransform(true);
        }
        else {
            btn->setNormalPic(":/images/resources/images/adapt_image_normal.png");
            btn->setHoverPic(":/images/resources/images/adapt_image_hover.png");
            btn->setPressPic(":/images/resources/images/adapt_image_active.png");
            btn->setToolTip(tr("1:1 Size"));
            btn->setWhatsThis("1:1SizeButton");
            emit resetTransform(false);
        }
    });
    connect(this, &TTMContent::imageEmpty, this, [=] (bool v) {
        btn->setDisabled(v);
        if (v) {
            btn->setNormalPic(":/images/resources/images/adapt_image_disable.png");
            btn->setHoverPic(":/images/resources/images/adapt_image_disable.png");
            btn->setPressPic(":/images/resources/images/adapt_image_disable.png");
        }
        else {
            btn->setNormalPic(":/images/resources/images/adapt_image_normal.png");
            btn->setHoverPic(":/images/resources/images/adapt_image_hover.png");
            btn->setPressPic(":/images/resources/images/adapt_image_active.png");
        }
    });

    if (! fromFileManager) {
        m_clBT = new ImageButton();
        hb->addWidget(m_clBT);
        connect(this, &TTMContent::imageEmpty, this, [=] (bool v) {
            m_clBT->setDisabled(v);
            updateCollectButton();
        });
        connect(m_clBT, &ImageButton::clicked, [=] {
            if (dbManager()->imageExistAlbum(m_imageName, FAVORITES_ALBUM)) {
                dbManager()->removeImageFromAlbum(FAVORITES_ALBUM,m_imageName);
            }
            else {
                DatabaseManager::ImageInfo info =
                        dbManager()->getImageInfoByName(m_imageName);
                dbManager()->insertImageIntoAlbum(FAVORITES_ALBUM,
                    m_imageName, utils::base::timeToString(info.time));
            }
            updateCollectButton();
        });
        updateCollectButton();
    }

    btn = new ImageButton();
    btn->setNormalPic(":/images/resources/images/contrarotate_normal.png");
    btn->setHoverPic(":/images/resources/images/contrarotate_hover.png");
    btn->setPressPic(":/images/resources/images/contrarotate_press.png");
    btn->setToolTip(tr("Rotate counterclockwise"));
    hb->addWidget(btn);
    connect(btn, &ImageButton::clicked, this, &TTMContent::rotateCounterClockwise);
    connect(this, &TTMContent::imageEmpty, this, [=] (bool v) {
        btn->setDisabled(v);
        if (v) {
            btn->setNormalPic(":/images/resources/images/contrarotate_disable.png");
            btn->setHoverPic(":/images/resources/images/contrarotate_disable.png");
            btn->setPressPic(":/images/resources/images/contrarotate_disable.png");
        }
        else {
            btn->setNormalPic(":/images/resources/images/contrarotate_normal.png");
            btn->setHoverPic(":/images/resources/images/contrarotate_hover.png");
            btn->setPressPic(":/images/resources/images/contrarotate_press.png");
        }
    });

    btn = new ImageButton();
    btn->setNormalPic(":/images/resources/images/clockwise_rotation_normal.png");
    btn->setHoverPic(":/images/resources/images/clockwise_rotation_hover.png");
    btn->setPressPic(":/images/resources/images/clockwise_rotation_press.png");
    btn->setToolTip(tr("Rotate clockwise"));
    hb->addWidget(btn);
    connect(btn, &ImageButton::clicked, this, &TTMContent::rotateClockwise);
    connect(this, &TTMContent::imageEmpty, this, [=] (bool v) {
        btn->setDisabled(v);
        if (v) {
            btn->setNormalPic(":/images/resources/images/clockwise_rotation_disable.png");
            btn->setHoverPic(":/images/resources/images/clockwise_rotation_disable.png");
            btn->setPressPic(":/images/resources/images/clockwise_rotation_disable.png");
        }
        else {
            btn->setNormalPic(":/images/resources/images/clockwise_rotation_normal.png");
            btn->setHoverPic(":/images/resources/images/clockwise_rotation_hover.png");
            btn->setPressPic(":/images/resources/images/clockwise_rotation_press.png");
        }
    });

    btn = new ImageButton();
    btn->setNormalPic(":/images/resources/images/delete_normal.png");
    btn->setHoverPic(":/images/resources/images/delete_hover.png");
    btn->setPressPic(":/images/resources/images/delete_press.png");
    btn->setToolTip(tr("Throw to Trash"));
    hb->addWidget(btn);
    connect(btn, &ImageButton::clicked, this, &TTMContent::removed);
    connect(this, &TTMContent::imageEmpty, this, [=] (bool v) {
        btn->setDisabled(v);
        if (v) {
            btn->setNormalPic(":/images/resources/images/delete_disable.png");
            btn->setHoverPic(":/images/resources/images/delete_disable.png");
            btn->setPressPic(":/images/resources/images/delete_disable.png");
        }
        else {
            btn->setNormalPic(":/images/resources/images/delete_normal.png");
            btn->setHoverPic(":/images/resources/images/delete_hover.png");
            btn->setPressPic(":/images/resources/images/delete_press.png");
        }
    });

    hb->addStretch();
}

void TTMContent::onImageChanged(const QString &name, const QString &path)
{
    m_imageName = name;
    m_imagePath = path;
    if (name.isEmpty())
        emit imageEmpty(true);
    else
        emit imageEmpty(false);
}

DatabaseManager *TTMContent::dbManager() const
{
    return DatabaseManager::instance();
}

void TTMContent::updateCollectButton()
{
    if (! m_clBT)
        return;
    if (! m_clBT->isEnabled()) {
        m_clBT->setNormalPic(":/images/resources/images/collect_disable.png");
        m_clBT->setHoverPic(":/images/resources/images/collect_disable.png");
        m_clBT->setPressPic(":/images/resources/images/collect_disable.png");
    }
    else if (dbManager()->imageExistAlbum(m_imageName, FAVORITES_ALBUM)) {
        m_clBT->setToolTip(tr("Unfavorite"));
        m_clBT->setWhatsThis("RFFButton");
        m_clBT->setNormalPic(":/images/resources/images/collect_active.png");
        m_clBT->setHoverPic(":/images/resources/images/collect_active.png");
        m_clBT->setPressPic(":/images/resources/images/collect_active.png");
    }
    else {
        m_clBT->setToolTip(tr("Add to My favorites"));
        m_clBT->setWhatsThis("ATFButton");
        m_clBT->setNormalPic(":/images/resources/images/collect_normal.png");
        m_clBT->setHoverPic(":/images/resources/images/collect_hover.png");
        m_clBT->setPressPic(":/images/resources/images/collect_hover.png");
    }
}
