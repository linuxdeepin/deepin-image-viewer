#include "ttlcontent.h"
#include "application.h"
#include "utils/baseutils.h"
#include "widgets/pushbutton.h"
#include <QHBoxLayout>
#include <QDebug>

namespace {

const int LEFT_MARGIN = 13;
const int MAX_BUTTON_WIDTH = 200;
const QString FAVORITES_ALBUM_NAME = "My favorites";
}  // namespace

TTLContent::TTLContent(bool inDB, QWidget *parent) : QWidget(parent)
{
    onThemeChanged(dApp->viewerTheme->getCurrentTheme());
    QHBoxLayout *hb = new QHBoxLayout(this);
    hb->setContentsMargins(LEFT_MARGIN, 0, 0, 0);
    hb->setSpacing(0);
    m_returnBtn = new PushButton();
    m_returnBtn->setMaximumWidth(MAX_BUTTON_WIDTH);
    m_returnBtn->setObjectName("ReturnBtn");
    m_returnBtn->setToolTip(tr("Back"));
    PushButton *folderBtn = new PushButton();
    folderBtn->setObjectName("FolderBtn");
    folderBtn->setToolTip(tr("Image management"));
    if(inDB) {
        hb->addWidget(m_returnBtn);
    } else {
       hb->addWidget(folderBtn);
    }
    hb->addStretch();

    connect(m_returnBtn, &PushButton::clicked, this, [=] {
        emit clicked();
    });
    connect(folderBtn, &PushButton::clicked, this, [=] {
        emit clicked();
    });
    connect(dApp->viewerTheme, &ViewerThemeManager::viewerThemeChanged,
            this, &TTLContent::onThemeChanged);
}

void TTLContent::onThemeChanged(ViewerThemeManager::AppTheme theme) {
    if (theme == ViewerThemeManager::Dark) {
        this->setStyleSheet(utils::base::getFileContent(
                                ":/resources/dark/qss/view.qss"));
    } else {
        this->setStyleSheet(utils::base::getFileContent(
                                ":/resources/light/qss/view.qss"));
    }
}

void TTLContent::setCurrentDir(QString text) {
    if (text == FAVORITES_ALBUM_NAME) {
        text = tr("My favorites");
    }
    m_returnBtn->setText(text);
    m_returnBtn->setMaximumWidth(this->width()/2);
    update();
}
