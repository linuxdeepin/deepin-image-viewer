#include "ttlcontent.h"
#include "widgets/imagebutton.h"
#include <QHBoxLayout>
TTLContent::TTLContent(ImageSource source, QWidget *parent) : QWidget(parent)
{
    QHBoxLayout *hb = new QHBoxLayout(this);
    hb->setContentsMargins(5, 0, 0, 0);
    hb->setSpacing(0);

    ImageButton *btn = new ImageButton();
    switch (source) {
    case FromAlbum:
    case FromTimeline:
        btn->setNormalPic(":/images/resources/images/return_normal.png");
        btn->setHoverPic(":/images/resources/images/return_hover.png");
        btn->setPressPic(":/images/resources/images/return_press.png");
        btn->setToolTip(tr("Return"));
        break;
    default:
        btn->setNormalPic(":/images/resources/images/folder_normal.png");
        btn->setHoverPic(":/images/resources/images/folder_hover.png");
        btn->setPressPic(":/images/resources/images/folder_press.png");
        btn->setToolTip(tr("Image management"));
        break;
    }
    hb->addWidget(btn);
    hb->addStretch();

    connect(btn, &ImageButton::clicked, this, [=] {
        emit clicked(source);
    });
}
