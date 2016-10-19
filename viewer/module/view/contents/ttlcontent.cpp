#include "ttlcontent.h"
#include "widgets/imagebutton.h"
#include <QHBoxLayout>

const int ICON_MARGIN = 13;
TTLContent::TTLContent(bool inDB, QWidget *parent) : QWidget(parent)
{
    QHBoxLayout *hb = new QHBoxLayout(this);
    hb->setContentsMargins(ICON_MARGIN, 0, 0, 0);
    hb->setSpacing(0);

    ImageButton *btn = new ImageButton();
    m_curDirLabel = new QLabel();
    m_curDirLabel->setObjectName("CurrentDirLabel");
    //TODO: Since there is only one label used the brief QSS,
    //so do not use read QSS file
    m_curDirLabel->setStyleSheet("QLabel#CurrentDirLabel{"
                                 "color: rgba(255, 255, 255, 235);"
                                 "font-size: 12px;"
                                 "text-align: center;}");
                                //上 右 下 左
    if(inDB) {
        btn->setNormalPic(":/images/resources/images/return_normal.png");
        btn->setHoverPic(":/images/resources/images/return_hover.png");
        btn->setPressPic(":/images/resources/images/return_press.png");
        btn->setToolTip(tr("Back"));
    }
    else {
        btn->setNormalPic(":/images/resources/images/folder_normal.png");
        btn->setHoverPic(":/images/resources/images/folder_hover.png");
        btn->setPressPic(":/images/resources/images/folder_press.png");
        btn->setToolTip(tr("Image management"));
    }

    hb->addWidget(btn);
    hb->addWidget(m_curDirLabel);

    connect(btn, &ImageButton::clicked, this, [=] {
        emit clicked();
    });
}


void TTLContent::setCurrentDir(QString text) {
    m_curDirLabel->setText(text);
    m_curDirLabel->sizeHint();
    update();
}
