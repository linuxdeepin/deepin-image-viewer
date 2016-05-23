#include "ttmcontent.h"
#include "widgets/imagebutton.h"
#include <QBoxLayout>

TTMContent::TTMContent(QWidget *parent) : QWidget(parent)
{
    QHBoxLayout *hb = new QHBoxLayout(this);
    hb->setContentsMargins(0, 0, 0, 0);
    hb->setSpacing(10);
    hb->addStretch();

    ImageButton *btn = new ImageButton();
    btn->setNormalPic(":/images/resources/images/contrarotate_normal.png");
    btn->setHoverPic(":/images/resources/images/contrarotate_hover.png");
    btn->setPressPic(":/images/resources/images/contrarotate_press.png");
    btn->setToolTip(tr("Rotate counterclockwise"));
    hb->addWidget(btn);
    connect(btn, &ImageButton::clicked,
            this, &TTMContent::rotateCounterclockwise);

    btn = new ImageButton();
    btn->setNormalPic(":/images/resources/images/clockwise_rotation_normal.png");
    btn->setHoverPic(":/images/resources/images/clockwise_rotation_hover.png");
    btn->setPressPic(":/images/resources/images/clockwise_rotation_press.png");
    hb->addWidget(btn);
    btn->setToolTip(tr("Rotate clockwise"));
    connect(btn, &ImageButton::clicked, this, &TTMContent::rotateClockWise);

    btn = new ImageButton();
    btn->setNormalPic(":/images/resources/images/adapt_image_normal.png");
    btn->setHoverPic(":/images/resources/images/adapt_image_hover.png");
    btn->setPressPic(":/images/resources/images/adapt_image_active.png");
    btn->setToolTip(tr("1:1 Size"));
    btn->setWhatsThis("1:1SizeButton");
    hb->addWidget(btn);
    hb->addStretch();
    connect(btn, &ImageButton::clicked, [this, btn](){
        if (btn->whatsThis() == "1:1SizeButton") {
            btn->setNormalPic(":/images/resources/images/adapt_screen_normal.png");
            btn->setHoverPic(":/images/resources/images/adapt_screen_hover.png");
            btn->setPressPic(":/images/resources/images/adapt_screen_active.png");
            btn->setToolTip(tr("Fit to window"));
            btn->setWhatsThis("FitToWindowButton");
            emit resetTransform(true);
        }
        else {
            btn->setNormalPic(":/images/resources/images/adapt_image_normal.png");
            btn->setHoverPic(":/images/resources/images/adapt_image_hover.png");
            btn->setPressPic(":/images/resources/images/adapt_image_active.png");
            btn->setToolTip(tr("1:1 Size"));
            btn->setWhatsThis("1:1SizeButton");
            emit resetTransform(false);
        }
    });
}
