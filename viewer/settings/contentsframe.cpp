#include "contentsframe.h"
#include "shortcut/shortcutframe.h"
#include "slideshow/slideshowframe.h"
#include <dimagebutton.h>
#include <dscrollbar.h>
#include <dthememanager.h>
#include <QCursor>
#include <QPropertyAnimation>
#include <QVBoxLayout>
#include <QStyleFactory>

namespace {

const int MAX_WIDTH = 520;
const int MAX_HEIGHT = 560;

// This is the simple way to map the scroll value to active title
// The settings window is set with fixed size
const QMap<TitleButton::SettingID, int> scrollMap()
{
    QMap<TitleButton::SettingID, int> m;
    m.insert(TitleButton::SlideshowSetting, 0);
    m.insert(TitleButton::SlideshowEffect, 30);
    m.insert(TitleButton::SlideshowTime, 170);
    m.insert(TitleButton::ShortcutSetting, 240);
    m.insert(TitleButton::ShortcutView, 273);
    m.insert(TitleButton::ShortcutAlbum, 535);
    return m;
}

TitleButton::SettingID scrollValueToFieldID(int value)
{
    auto sm = scrollMap();
    if (value >= sm[TitleButton::ShortcutAlbum])
        return TitleButton::ShortcutAlbum;
    else if (value >= sm[TitleButton::ShortcutView])
        return TitleButton::ShortcutView;
    else if (value >= sm[TitleButton::ShortcutSetting])
        return TitleButton::ShortcutSetting;
    else if (value >= sm[TitleButton::SlideshowTime])
        return TitleButton::SlideshowTime;
    else if (value >= sm[TitleButton::SlideshowEffect])
        return TitleButton::SlideshowEffect;
    else
        return TitleButton::SlideshowSetting;
}

int fieldIDToScrollValue(TitleButton::SettingID id)
{
    return scrollMap().value(id);
}

}  // namespace

using namespace Dtk::Widget;

ContentsFrame::ContentsFrame(QWidget *parent)
    : QWidget(parent)
{
    setFixedSize(MAX_WIDTH, MAX_HEIGHT);

    m_layout = new QVBoxLayout(this);
    m_layout->setContentsMargins(0, 0, 0, 0);
    m_layout->setSpacing(0);

    // Close button
    DImageButton *cb = new DImageButton(":/settings/images/resources/images/close_normal.png",
                                        ":/settings/images/resources/images/close_hover.png",
                                        ":/settings/images/resources/images/close_press.png");
    connect(cb, &DImageButton::clicked, parent->window(), &QWidget::hide);
    QFrame *cbf = new QFrame;
    cbf->setObjectName("CloseButtonFrame");
    cbf->setContentsMargins(0, 0, 0, 0);
    QHBoxLayout *tbl = new QHBoxLayout(cbf);
    tbl->setContentsMargins(0, 3, 3, 0);
    tbl->setSpacing(0);
    tbl->addStretch();
    tbl->addWidget(cb);
    m_layout->addWidget(cbf);

    // Scroll area
    initScrollArea();
}

void ContentsFrame::setCurrentID(const TitleButton::SettingID id)
{
    QPropertyAnimation *animation =
            new QPropertyAnimation(m_area->verticalScrollBar(), "value");
    animation->setDuration(500);
    animation->setEasingCurve(QEasingCurve::OutCubic);
    animation->setStartValue(m_area->verticalScrollBar()->value());
    animation->setEndValue(fieldIDToScrollValue(id));
    animation->start();
    connect(animation, &QPropertyAnimation::finished,
            animation, &QPropertyAnimation::deleteLater);
}

void ContentsFrame::initScrollArea()
{
    m_area = new QScrollArea;
    QScrollBar* sb = new QScrollBar();
    sb->setStyle(QStyleFactory::create("dlight"));
    m_area->setVerticalScrollBar(sb);
    sb->setContextMenuPolicy(Qt::PreventContextMenu);
    connect(sb, &DScrollBar::valueChanged,
            this, [=] (int value) {
        QRect gr(this->mapToGlobal(QPoint(0, 0)), rect().size());
        if (gr.contains(QCursor::pos())) {
            emit currentFieldChanged(scrollValueToFieldID(value));
        }
    });

    m_area->setFixedWidth(MAX_WIDTH);
    m_area->setContentsMargins(0, 0, 0, 0);
    m_area->setWidgetResizable(true);
    m_area->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    QWidget *content = new QWidget;
    QVBoxLayout *layout = new QVBoxLayout(content);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);

    SlideshowFrame *slideshow = new SlideshowFrame(this);
    ShortcutFrame *shortcut = new ShortcutFrame(this);
    connect(shortcut, &ShortcutFrame::resetAll,
            slideshow, &SlideshowFrame::requestReset);

    layout->addWidget(slideshow);
    layout->addWidget(shortcut);
    layout->addStretch();

    m_area->setWidget(content);

    m_layout->addWidget(m_area);
}
