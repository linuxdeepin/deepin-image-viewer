#include "ttmcontent.h"
#include "application.h"
#include "controller/dbmanager.h"
#include "utils/baseutils.h"
#include "utils/imageutils.h"
#include "widgets/pushbutton.h"
#include <QBoxLayout>
#include <QDebug>
#include <QFileInfo>

namespace {

const QString FAVORITES_ALBUM = "My favorite";
const int MARGIN_DIFF = 82;
const QSize ICON_SIZE = QSize(48, 39);
}  // namespace

TTMContent::TTMContent(bool fromFileManager, QWidget *parent)
    : QWidget(parent)
{
    onThemeChanged(dApp->viewerTheme->getCurrentTheme());
    m_layout = new QHBoxLayout(this);
    m_layout->setContentsMargins(0, 0, 0, 0);
    m_layout->setSpacing(0);
    m_layout->addStretch();
    // Adapt buttons////////////////////////////////////////////////////////////
    m_adaptImageBtn = new PushButton();
    m_adaptImageBtn->setObjectName("AdaptBtn");
    m_adaptImageBtn->setFixedSize(ICON_SIZE);

    m_adaptImageBtn->setToolTip(tr("1:1 Size"));
    m_layout->addWidget(m_adaptImageBtn);
    connect(m_adaptImageBtn, &PushButton::clicked, this, [=] {
        emit resetTransform(false);
    });

    m_adaptScreenBtn = new PushButton();
    m_adaptScreenBtn->setFixedSize(ICON_SIZE);
    m_adaptScreenBtn->setObjectName("AdaptScreenBtn");
    m_adaptScreenBtn->setToolTip(tr("Fit to window"));
    m_layout->addWidget(m_adaptScreenBtn);
    connect(m_adaptScreenBtn, &PushButton::clicked, this, [=] {
        emit resetTransform(true);
    });


    // Collection button////////////////////////////////////////////////////////
    m_clBT = new PushButton();
    m_clBT->setObjectName("CollectBtn");
    if (! fromFileManager) {

        m_layout->addWidget(m_clBT);
        connect(m_clBT, &PushButton::clicked, this, [=] {
            if (dApp->dbM->isImgExistInAlbum(FAVORITES_ALBUM, m_imagePath)) {
                dApp->dbM->removeFromAlbum(FAVORITES_ALBUM, QStringList(m_imagePath));
            }
            else {
                dApp->dbM->insertIntoAlbum(FAVORITES_ALBUM, QStringList(m_imagePath));
            }
            updateCollectButton();
        });
        updateCollectButton();
    }

    m_rotateLBtn = new PushButton();
    m_rotateLBtn->setFixedSize(ICON_SIZE);
    m_rotateLBtn->setObjectName("RotateBtn");
    m_rotateLBtn->setToolTip(tr("Rotate counterclockwise"));
    m_layout->addWidget(m_rotateLBtn);
    connect(m_rotateLBtn, &PushButton::clicked,
            this, &TTMContent::rotateCounterClockwise);

    m_rotateRBtn = new PushButton();
    m_rotateRBtn->setFixedSize(ICON_SIZE);
    m_rotateRBtn->setObjectName("RotateCounterBtn");
    m_rotateRBtn->setToolTip(tr("Rotate clockwise"));
    m_layout->addWidget(m_rotateRBtn);
    connect(m_rotateRBtn, &PushButton::clicked,
            this, &TTMContent::rotateClockwise);

    m_trashBtn = new PushButton();
    m_trashBtn->setFixedSize(ICON_SIZE);
    m_trashBtn->setObjectName("TrashBtn");
    m_trashBtn->setToolTip(tr("Throw to Trash"));
    m_layout->addWidget(m_trashBtn);
    m_layout->addStretch();

    connect(m_trashBtn, &PushButton::clicked, this, &TTMContent::removed);
    connect(dApp->viewerTheme, &ViewerThemeManager::viewerThemeChanged, this,
            &TTMContent::onThemeChanged);
}

void TTMContent::onThemeChanged(ViewerThemeManager::AppTheme theme) {
    if (theme == ViewerThemeManager::Dark) {
        this->setStyleSheet(utils::base::getFileContent(
                                ":/resources/dark/qss/ttm.qss"));
    } else {
        this->setStyleSheet(utils::base::getFileContent(
                                ":/resources/light/qss/ttm.qss"));
    }
}
void TTMContent::setImage(const QString &path)
{
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

    if (m_imagePath.isEmpty()) {
        m_clBT->setDisabled(true);
        m_clBT->setChecked(false);
    }
    else
        m_clBT->setDisabled(false);

    if (! m_clBT->isEnabled()) {
        m_clBT->setDisabled(true);
    }
    else if (dApp->dbM->isImgExistInAlbum(FAVORITES_ALBUM, m_imagePath)) {
        m_clBT->setToolTip(tr("Remove from my favorite"));
        m_clBT->setChecked(true);
    }
    else {
        m_clBT->setToolTip(tr("Add to my favorite"));
        m_clBT->setChecked(false);
        m_clBT->setDisabled(false);
    }
}

