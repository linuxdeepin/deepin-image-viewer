#include "ttmcontent.h"
#include "application.h"
#include "controller/databasemanager.h"
#include "utils/baseutils.h"
#include "utils/imageutils.h"
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
    m_layout = new QHBoxLayout(this);
    m_layout->setContentsMargins(0, 0, 0, 0);
    m_layout->setSpacing(0);
    m_layout->addSpacing(MARGIN_DIFF);

    const QString res = ":/images/resources/images/";

    // Adapt buttons////////////////////////////////////////////////////////////
    m_adaptImageBtn = new ImageButton(res + "adapt_image_normal.png"
                                      , res + "adapt_image_hover.png"
                                      , res + "adapt_image_press.png"
                                      , res + "adapt_image_disable.png");
    m_adaptImageBtn->setToolTip(tr("1:1 Size"));
    m_layout->addWidget(m_adaptImageBtn);
    connect(m_adaptImageBtn, &ImageButton::clicked, [this](){
        emit resetTransform(false);
    });

    m_adaptScreenBtn = new ImageButton(res + "adapt_screen_normal.png"
                                       , res + "adapt_screen_hover.png"
                                       , res + "adapt_screen_press.png"
                                       , res + "adapt_screen_disable.png");
    m_adaptScreenBtn->setToolTip(tr("Fit to window"));
    m_layout->addWidget(m_adaptScreenBtn);
    connect(m_adaptScreenBtn, &ImageButton::clicked, [this](){
        emit resetTransform(true);
    });


    // Collection button////////////////////////////////////////////////////////
    if (! fromFileManager) {
        m_clBT = new ImageButton();
        m_layout->addWidget(m_clBT);
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

    m_rotateRBtn = new ImageButton(res + "contrarotate_normal.png"
                                   , res + "contrarotate_hover.png"
                                   , res + "contrarotate_press.png"
                                   , res + "contrarotate_disable.png");
    m_rotateRBtn->setToolTip(tr("Rotate clockwise"));
    m_layout->addWidget(m_rotateRBtn);
    connect(m_rotateRBtn, &ImageButton::clicked,
            this, &TTMContent::rotateClockwise);

    m_rotateLBtn = new ImageButton(res + "clockwise_rotation_normal.png"
                                   , res + "clockwise_rotation_hover.png"
                                   , res + "clockwise_rotation_press.png"
                                   , res + "clockwise_rotation_disable.png");
    m_rotateLBtn->setToolTip(tr("Rotate counterclockwise"));
    m_layout->addWidget(m_rotateLBtn);
    connect(m_rotateLBtn, &ImageButton::clicked,
            this, &TTMContent::rotateCounterClockwise);

    m_trashBtn = new ImageButton(res + "delete_normal.png"
                                 , res + "delete_hover.png"
                                 , res + "delete_press.png"
                                 , res + "delete_disable.png");
    m_trashBtn->setToolTip(tr("Throw to Trash"));
    m_layout->addWidget(m_trashBtn);
    connect(m_trashBtn, &ImageButton::clicked, this, &TTMContent::removed);
}

void TTMContent::onImageChanged(const QString &path)
{
    m_imageName = QFileInfo(path).fileName();
    m_imagePath = path;
    if (path.isEmpty()) {
        m_adaptImageBtn->setDisabled(true);
        m_adaptScreenBtn->setDisabled(true);
        m_rotateLBtn->setDisabled(true);
        m_rotateRBtn->setDisabled(true);
        m_trashBtn->setDisabled(true);
    }
    else {
        m_adaptImageBtn->setDisabled(false);
        m_adaptScreenBtn->setDisabled(false);
        m_trashBtn->setDisabled(false);
        if (utils::image::imageSupportSave(path)) {
            m_rotateLBtn->setDisabled(false);
            m_rotateRBtn->setDisabled(false);
        }
        else {
            m_rotateLBtn->setDisabled(true);
            m_rotateRBtn->setDisabled(true);
        }
    }

    updateCollectButton();
}

void TTMContent::updateCollectButton()
{
    if (! m_clBT)
        return;

    if (m_imagePath.isEmpty())
        m_clBT->setDisabled(true);
    else
        m_clBT->setDisabled(false);

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

