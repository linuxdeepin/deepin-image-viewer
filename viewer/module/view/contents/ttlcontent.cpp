#include "ttlcontent.h"
#include "application.h"
#include "utils/baseutils.h"
#include "widgets/imagebutton.h"
#include <QHBoxLayout>
#include <QDebug>

const int ICON_MARGIN = 13;
TTLContent::TTLContent(bool inDB, QWidget *parent) : QWidget(parent)
{
    onThemeChanged(dApp->viewerTheme->getCurrentTheme());
    QHBoxLayout *hb = new QHBoxLayout(this);
    hb->setContentsMargins(0, 0, 0, 0);
    hb->setSpacing(0);
    hb->addSpacing(ICON_MARGIN);
    ImageButton *returnBtn = new ImageButton();
    returnBtn->setObjectName("ReturnBtn");
    returnBtn->setToolTip(tr("Back"));
    ImageButton *folderBtn = new ImageButton();
    folderBtn->setObjectName("FolderBtn");
    folderBtn->setToolTip(tr("Image management"));
    if(inDB) {
        hb->addWidget(returnBtn);
    } else {
       hb->addWidget(folderBtn);
    }

    m_curDirLabel = new QLabel(this);
    m_curDirLabel->setObjectName("CurrentDirLabel");

    hb->addWidget(m_curDirLabel);
    hb->addStretch();

    connect(returnBtn, &ImageButton::clicked, this, [=] {
        emit clicked();
    });
    connect(folderBtn, &ImageButton::clicked, this, [=] {
        emit clicked();
    });
    connect(dApp->viewerTheme, &ViewerThemeManager::viewerThemeChanged, this,
            &TTLContent::onThemeChanged);
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
    QString dir = m_curDirLabel->fontMetrics().elidedText(text, Qt::ElideMiddle,
                                                          this->width()/2);
    m_curDirLabel->setText(dir);
    update();
}
