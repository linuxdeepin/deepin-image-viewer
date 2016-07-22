#include "ttlcontent.h"
#include "widgets/imagebutton.h"
#include <QHBoxLayout>
TTLContent::TTLContent(bool inDB, QWidget *parent) : QWidget(parent)
{
    QHBoxLayout *hb = new QHBoxLayout(this);
    hb->setContentsMargins(9, 0, 0, 0);
    hb->setSpacing(0);

    ImageButton *btn = new ImageButton();
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
    hb->addStretch();

    connect(btn, &ImageButton::clicked, this, [=] {
        emit clicked();
    });
}
