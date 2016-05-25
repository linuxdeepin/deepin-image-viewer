#include "ttlcontent.h"
#include "widgets/imagebutton.h"
#include <QHBoxLayout>
TTLContent::TTLContent(QWidget *parent) : QWidget(parent)
{
    QHBoxLayout *hb = new QHBoxLayout(this);
    hb->setContentsMargins(5, 0, 0, 0);
    hb->setSpacing(0);

    ImageButton *btn = new ImageButton();
    btn->setNormalPic(":/images/resources/images/folder_normal.png");
    btn->setHoverPic(":/images/resources/images/folder_hover.png");
    btn->setPressPic(":/images/resources/images/folder_press.png");
    hb->addWidget(btn);
    connect(btn, &ImageButton::clicked, this, [=] {
        emit backToMain();
    });
    btn->setToolTip(tr("Image management"));
}
