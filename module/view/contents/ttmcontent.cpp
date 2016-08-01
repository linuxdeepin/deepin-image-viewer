#include "ttmcontent.h"
#include "application.h"
#include "controller/databasemanager.h"
#include "utils/baseutils.h"
#include "widgets/imagebutton.h"
#include <QBoxLayout>
#include <QDebug>
#include <QFileInfo>

namespace {

const QString FAVORITES_ALBUM = "My favorites";

}  // namespace

TTMContent::TTMContent(bool fromFileManager, QWidget *parent)
    : QWidget(parent)
{
    QHBoxLayout *hb = new QHBoxLayout(this);
    hb->setContentsMargins(0, 0, 0, 0);
    hb->setSpacing(21);
    hb->addStretch();

    m_adaptButton = new ImageButton();
    updateAdaptButton(true);
    hb->addWidget(m_adaptButton);
    connect(m_adaptButton, &ImageButton::clicked, [this](){
        if (m_adaptButton->whatsThis() != "1:1SizeButton") {
            updateAdaptButton(true);
            emit resetTransform(false);
        }
        else {
            updateAdaptButton(false);
            emit resetTransform(true);
        }
    });
    connect(this, &TTMContent::imageEmpty, this, [=] (bool v) {
        m_adaptButton->setDisabled(v);
    });

    if (! fromFileManager) {
        m_clBT = new ImageButton();
        hb->addWidget(m_clBT);
        connect(this, &TTMContent::imageEmpty, this, [=] (bool v) {
            m_clBT->setDisabled(v);
            updateCollectButton();
        });
        connect(m_clBT, &ImageButton::clicked, [=] {
            if (dApp->databaseM->imageExistAlbum(m_imageName, FAVORITES_ALBUM)) {
                dApp->databaseM->removeImageFromAlbum(FAVORITES_ALBUM,m_imageName);
            }
            else {
                auto info =
                        dApp->databaseM->getImageInfoByName(m_imageName);
                dApp->databaseM->insertImageIntoAlbum(FAVORITES_ALBUM,
                    m_imageName, utils::base::timeToString(info.time));
            }
            updateCollectButton();
        });
        updateCollectButton();
    }

    ImageButton *btn = new ImageButton();
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

void TTMContent::onImageChanged(const QString &path, bool adaptScreen)
{
    m_imageName = QFileInfo(path).fileName();
    m_imagePath = path;
    if (path.isEmpty())
        emit imageEmpty(true);
    else
        emit imageEmpty(false);

    this->updateAdaptButton(adaptScreen);
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
    else if (dApp->databaseM->imageExistAlbum(m_imageName, FAVORITES_ALBUM)) {
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

void TTMContent::updateAdaptButton(bool v)
{
    if (! m_adaptButton->isEnabled()) {
        m_adaptButton->setNormalPic(":/images/resources/images/adapt_image_disable.png");
        m_adaptButton->setHoverPic(":/images/resources/images/adapt_image_disable.png");
        m_adaptButton->setPressPic(":/images/resources/images/adapt_image_disable.png");
        return;
    }
    // Change the icon and tooltip for next state
    if (v) {
        m_adaptButton->setNormalPic(":/images/resources/images/adapt_image_normal.png");
        m_adaptButton->setHoverPic(":/images/resources/images/adapt_image_hover.png");
        m_adaptButton->setPressPic(":/images/resources/images/adapt_image_active.png");
        m_adaptButton->setToolTip(tr("1:1 Size"));
        m_adaptButton->setWhatsThis("1:1SizeButton");
    }
    else {
        m_adaptButton->setNormalPic(":/images/resources/images/adapt_screen_normal.png");
        m_adaptButton->setHoverPic(":/images/resources/images/adapt_screen_hover.png");
        m_adaptButton->setPressPic(":/images/resources/images/adapt_screen_active.png");
        m_adaptButton->setToolTip(tr("Fit to window"));
        m_adaptButton->setWhatsThis("FitToWindowButton");
    }
}
