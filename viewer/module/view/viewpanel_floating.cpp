#include "viewpanel.h"
#include "navigationwidget.h"
#include "contents/imageinfowidget.h"
#include "scen/imageview.h"
#include "widgets/imagebutton.h"
#include <QTimer>

DWIDGET_USE_NAMESPACE

void ViewPanel::initFloatingComponent()
{
    initSwitchButtons();
    initScaleLabel();
    initNavigation();
}

void ViewPanel::initSwitchButtons()
{
    Anchors<ImageButton> preButton = new ImageButton(this);
    preButton->setTooltipVisible(true);
    preButton->setObjectName("PreviousButton");
    preButton->setNormalPic(":/images/resources/images/previous_hover.png");
    preButton->setHoverPic(":/images/resources/images/previous_hover.png");
    preButton->setPressPic(":/images/resources/images/previous_press.png");
    preButton.setAnchor(Qt::AnchorVerticalCenter, this, Qt::AnchorVerticalCenter);
    // The preButton is anchored to the left of this
    preButton.setAnchor(Qt::AnchorLeft, this, Qt::AnchorLeft);
    // NOTE: this is a bug of Anchors,the button should be resize after set anchor
    preButton->resize(53, 53);
    preButton.setLeftMargin(20);
    preButton->hide();
    connect(preButton, &ImageButton::clicked, this, &ViewPanel::showPrevious);

    Anchors<ImageButton> nextButton = new ImageButton(this);
    nextButton->setTooltipVisible(true);
    nextButton->setObjectName("NextButton");
    nextButton->setNormalPic(":/images/resources/images/next_hover.png");
    nextButton->setHoverPic(":/images/resources/images/next_hover.png");
    nextButton->setPressPic(":/images/resources/images/next_press.png");
    nextButton.setAnchor(Qt::AnchorVerticalCenter, this, Qt::AnchorVerticalCenter);
    nextButton.setAnchor(Qt::AnchorRight, this, Qt::AnchorRight);
    nextButton->setFixedSize(53, 53);
    nextButton.setRightMargin(20);
    nextButton->hide();
    connect(nextButton, &ImageButton::clicked, this, &ViewPanel::showNext);

    connect(m_viewB, &ImageView::mouseHoverMoved, this, [=] {
        const int EXTEND_SPACING = 15;

        Anchors<ImageButton> pb = preButton;
        if (m_info->visibleRegion().isNull()) {
            pb.setLeftMargin(20);
        }
        else {
            pb.setLeftMargin(260);
        }

        const QPoint pp = preButton->mapToGlobal(QPoint(0, 0))
                - QPoint(EXTEND_SPACING, EXTEND_SPACING);
        QRect pr(pp, QSize(preButton->width() + EXTEND_SPACING * 2,
                           preButton->height() + EXTEND_SPACING * 2));

        const QPoint np = nextButton->mapToGlobal(QPoint(0, 0))
                - QPoint(EXTEND_SPACING, EXTEND_SPACING);
        QRect nr(np, QSize(nextButton->width() + EXTEND_SPACING * 2,
                           nextButton->height() + EXTEND_SPACING * 2));

        if (pr.contains(QCursor::pos()) || nr.contains(QCursor::pos())) {
            preButton->show();
            nextButton->show();
        }
        else {
            preButton->hide();
            nextButton->hide();
        }
    });
}

void ViewPanel::initScaleLabel()
{
        Anchors<QLabel> scalePerc = new QLabel(this);
        scalePerc->setObjectName("ScaleLabel");
        scalePerc->setAttribute(Qt::WA_TransparentForMouseEvents);
        scalePerc->setAlignment(Qt::AlignCenter);
        scalePerc.setAnchor(Qt::AnchorVerticalCenter, this, Qt::AnchorVerticalCenter);
        scalePerc.setAnchor(Qt::AnchorHorizontalCenter, this, Qt::AnchorHorizontalCenter);
        scalePerc->setFixedSize(82, 48);
        scalePerc->setText("100%");
        scalePerc->hide();

        QTimer *hideT = new QTimer(this);
        hideT->setSingleShot(true);
        connect(hideT, &QTimer::timeout, scalePerc, &QLabel::hide);

        connect(m_viewB, &ImageView::scaled, this, [=](qreal perc) {
            QString originText = scalePerc->text();
            scalePerc->setText(QString("%1%").arg(int(perc)));
            if (scalePerc->text() != originText) {
                scalePerc->show();
                hideT->start(2000);
            }
        });
}

void ViewPanel::initNavigation()
{
    m_nav = new NavigationWidget(this);
    m_nav.setAnchor(Qt::AnchorRight, this, Qt::AnchorRight);
    m_nav.setAnchor(Qt::AnchorBottom, this, Qt::AnchorBottom);

    m_nav->setVisible(! m_nav->isAlwaysHidden());
    connect(this, &ViewPanel::imageChanged, this, [=] (const QString &path) {
        if (path.isEmpty()) m_nav->setVisible(false);
        m_nav->setImage(m_viewB->image());
    });
    connect(m_nav, &NavigationWidget::requestMove, [this](int x, int y){
        m_viewB->centerOn(x, y);
    });
    connect(m_viewB, &ImageView::transformChanged, [this](){
        m_nav->setVisible(! m_nav->isAlwaysHidden() && ! m_viewB->isWholeImageVisible());
        m_nav->setRectInImage(m_viewB->visibleImageRect());
    });
}
