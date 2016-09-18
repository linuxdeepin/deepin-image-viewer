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
const int MARGIN_DIFF = 82;
}  // namespace

TTMContent::TTMContent(bool fromFileManager, QWidget *parent)
    : QWidget(parent)
{
    QHBoxLayout *hb = new QHBoxLayout(this);
    hb->setContentsMargins(0, 0, 0, 0);
    hb->setSpacing(0);
    hb->addSpacing(MARGIN_DIFF);

    const QString res = ":/images/resources/images/";

    // Adapt buttons////////////////////////////////////////////////////////////
    ImageButton *btn = new ImageButton(res + "adapt_image_normal.png"
                                       , res + "adapt_image_hover.png"
                                       , res + "adapt_image_press.png"
                                       , res + "adapt_image_disable.png");
    btn->setToolTip(tr("1:1 Size"));
    hb->addWidget(btn);
    connect(this, &TTMContent::imageEmpty, btn, &ImageButton::setDisabled);
    connect(btn, &ImageButton::clicked, [this](){
        emit resetTransform(false);
    });

    btn = new ImageButton(res + "adapt_screen_normal.png"
                          , res + "adapt_screen_hover.png"
                          , res + "adapt_screen_press.png"
                          , res + "adapt_screen_disable.png");
    btn->setToolTip(tr("Fit to window"));
    hb->addWidget(btn);
    connect(this, &TTMContent::imageEmpty, btn, &ImageButton::setDisabled);
    connect(btn, &ImageButton::clicked, [this](){
        emit resetTransform(true);
    });


    // Collection button////////////////////////////////////////////////////////
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
                auto info = dApp->databaseM->getImageInfoByName(m_imageName);
                dApp->databaseM->insertImageIntoAlbum(FAVORITES_ALBUM,
                    m_imageName, utils::base::timeToString(info.time));
            }
            updateCollectButton();
        });
        updateCollectButton();
    }

    btn = new ImageButton();
    btn = new ImageButton(res + "contrarotate_normal.png"
                          , res + "contrarotate_hover.png"
                          , res + "contrarotate_press.png"
                          , res + "contrarotate_disable.png");
    btn->setToolTip(tr("Rotate clockwise"));
    hb->addWidget(btn);
    connect(btn, &ImageButton::clicked, this, &TTMContent::rotateClockwise);
    connect(this, &TTMContent::imageEmpty, btn, &ImageButton::setDisabled);

    btn = new ImageButton(res + "clockwise_rotation_normal.png"
                          , res + "clockwise_rotation_hover.png"
                          , res + "clockwise_rotation_press.png"
                          , res + "clockwise_rotation_disable.png");
    btn->setToolTip(tr("Rotate counterclockwise"));
    hb->addWidget(btn);
    connect(btn, &ImageButton::clicked, this, &TTMContent::rotateCounterClockwise);
    connect(this, &TTMContent::imageEmpty, btn, &ImageButton::setDisabled);

    btn = new ImageButton(res + "delete_normal.png"
                          , res + "delete_hover.png"
                          , res + "delete_press.png"
                          , res + "delete_disable.png");
    btn->setToolTip(tr("Throw to Trash"));
    hb->addWidget(btn);
    connect(btn, &ImageButton::clicked, this, &TTMContent::removed);
    connect(this, &TTMContent::imageEmpty, btn, &ImageButton::setDisabled);
}

void TTMContent::onImageChanged(const QString &path)
{
    m_imageName = QFileInfo(path).fileName();
    m_imagePath = path;
    if (path.isEmpty())
        emit imageEmpty(true);
    else
        emit imageEmpty(false);
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
        m_clBT->setNormalPic(":/images/resources/images/collect_press.png");
        m_clBT->setHoverPic(":/images/resources/images/collect_press.png");
        m_clBT->setPressPic(":/images/resources/images/collect_press.png");
    }
    else {
        m_clBT->setToolTip(tr("Add to My favorites"));
        m_clBT->setNormalPic(":/images/resources/images/collect_normal.png");
        m_clBT->setHoverPic(":/images/resources/images/collect_hover.png");
        m_clBT->setPressPic(":/images/resources/images/collect_hover.png");
    }
}

