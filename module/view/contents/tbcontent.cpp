#include "tbcontent.h"
#include "controller/signalmanager.h"
#include "controller/databasemanager.h"
#include "widgets/imagebutton.h"
#include "utils/imageutils.h"
#include <QHBoxLayout>

namespace {

const QString FAVORITES_ALBUM_NAME = "My favorites";

}  // namespace

TBContent::TBContent(QWidget *parent)
    : QWidget(parent),
      m_signalManager(SignalManager::instance()),
      m_dbManager(DatabaseManager::instance())
{
    QHBoxLayout *hb = new QHBoxLayout(this);
    hb->setContentsMargins(0, 0, 0, 0);
    hb->setSpacing(10);

    ImageButton *btn = new ImageButton();
    btn->setNormalPic(":/images/resources/images/info_normal.png");
    btn->setHoverPic(":/images/resources/images/info_hover.png");
    btn->setPressPic(":/images/resources/images/info_active.png");
    btn->setToolTip(tr("Image info"));
    hb->addWidget(btn);
    hb->addStretch();
    connect(btn, &ImageButton::clicked,
            m_signalManager, &SignalManager::showExtensionPanel);

    m_clBT = new ImageButton();
    hb->addWidget(m_clBT);
    connect(m_clBT, &ImageButton::clicked, [=] {
        if (m_dbManager->imageExistAlbum(m_imageName, FAVORITES_ALBUM_NAME)) {
            m_dbManager->removeImageFromAlbum(FAVORITES_ALBUM_NAME,m_imageName);
        }
        else {
            DatabaseManager::ImageInfo info =
                    m_dbManager->getImageInfoByName(m_imageName);
            m_dbManager->insertImageIntoAlbum(FAVORITES_ALBUM_NAME,
                m_imageName, utils::base::timeToString(info.time));
        }
        updateCollectButton();
    });
    updateCollectButton();

    btn = new ImageButton();
    btn->setNormalPic(":/images/resources/images/previous_normal.png");
    btn->setHoverPic(":/images/resources/images/previous_hover.png");
    btn->setPressPic(":/images/resources/images/previous_press.png");
    btn->setToolTip(tr("Previous"));
    hb->addWidget(btn);
    connect(btn, &ImageButton::clicked, this, &TBContent::showPrevious);

    btn = new ImageButton();
    btn->setNormalPic(":/images/resources/images/slideshow_normal.png");
    btn->setHoverPic(":/images/resources/images/slideshow_hover.png");
    btn->setPressPic(":/images/resources/images/slideshow_press.png");
    btn->setToolTip(tr("Slide show"));
    hb->addWidget(btn);
    connect(btn, &ImageButton::clicked, this, &TBContent::toggleSlideShow);

    btn = new ImageButton();
    btn->setNormalPic(":/images/resources/images/next_normal.png");
    btn->setHoverPic(":/images/resources/images/next_hover.png");
    btn->setPressPic(":/images/resources/images/next_press.png");
    btn->setToolTip(tr("Next"));
    hb->addWidget(btn);
    connect(btn, &ImageButton::clicked, this, &TBContent::showNext);

    btn = new ImageButton();
    btn->setNormalPic(":/images/resources/images/edit_normal.png");
    btn->setHoverPic(":/images/resources/images/edit_hover.png");
    btn->setPressPic(":/images/resources/images/edit_press.png");
    btn->setToolTip(tr("Edit"));
    hb->addWidget(btn);
    hb->addStretch();
    connect(btn, &ImageButton::clicked, [this](){
        Q_EMIT m_signalManager->editImage(m_imagePath);
    });

    btn = new ImageButton();
    btn->setNormalPic(":/images/resources/images/delete_normal.png");
    btn->setHoverPic(":/images/resources/images/delete_hover.png");
    btn->setPressPic(":/images/resources/images/delete_press.png");
    btn->setToolTip(tr("Delete"));
    hb->addWidget(btn);
    connect(btn, &ImageButton::clicked, [this](){
        m_dbManager->removeImage(m_imageName);
        //Todo: show next image, if next image is not exists, show previous one.
        //Q_EMIT showNext();
    });

}

void TBContent::updateCollectButton()
{
    if (m_dbManager->imageExistAlbum(m_imageName, FAVORITES_ALBUM_NAME)) {
        m_clBT->setToolTip(tr("Remove from Favorites"));
        m_clBT->setWhatsThis("RFFButton");
        m_clBT->setNormalPic(":/images/resources/images/collect_active.png");
        m_clBT->setHoverPic(":/images/resources/images/collect_active.png");
        m_clBT->setPressPic(":/images/resources/images/collect_active.png");
    }
    else {
        m_clBT->setToolTip(tr("Add to Favorites"));
        m_clBT->setWhatsThis("ATFButton");
        m_clBT->setNormalPic(":/images/resources/images/collect_normal.png");
        m_clBT->setHoverPic(":/images/resources/images/collect_hover.png");
        m_clBT->setPressPic(":/images/resources/images/collect_hover.png");
    }
}

void TBContent::onImageChanged(const QString &name, const QString &path)
{
    m_imageName = name;
    m_imagePath = path;
}
