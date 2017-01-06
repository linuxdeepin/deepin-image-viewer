#include "titleframe.h"
#include <QPainter>
#include <QVBoxLayout>

namespace {

const int MAX_WIDTH = 160;
const int MAX_HEIGHT = 560;

}

TitleFrame::TitleFrame(QWidget *parent)
    : QFrame(parent)
{
    setFixedSize(MAX_WIDTH, MAX_HEIGHT);

    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 40, 0, 0);
    layout->setSpacing(0);

    // Slidershow
    TitleButton *btn = new TitleButton(TitleButton::SlideshowSetting, true, tr("Slideshow"));
    btn->setIsActived(true);
    connect(btn, &TitleButton::clicked, this, &TitleFrame::onButtonClicked);
    layout->addWidget(btn);
    m_buttonList << btn;
    btn = new TitleButton(TitleButton::SlideshowEffect, false, tr("Effect"));
    connect(btn, &TitleButton::clicked, this, &TitleFrame::onButtonClicked);
    layout->addWidget(btn);
    m_buttonList << btn;
    btn = new TitleButton(TitleButton::SlideshowTime, false, tr("Duration"));
    connect(btn, &TitleButton::clicked, this, &TitleFrame::onButtonClicked);
    layout->addWidget(btn);
    m_buttonList << btn;

    // Shortcut
    btn = new TitleButton(TitleButton::ShortcutSetting, true, tr("Shortcuts"));
    connect(btn, &TitleButton::clicked, this, &TitleFrame::onButtonClicked);
    layout->addWidget(btn);
    m_buttonList << btn;
    btn = new TitleButton(TitleButton::ShortcutView, false, tr("View"));
    connect(btn, &TitleButton::clicked, this, &TitleFrame::onButtonClicked);
    layout->addWidget(btn);
    m_buttonList << btn;
    btn = new TitleButton(TitleButton::ShortcutAlbum, false, tr("Album"));
    connect(btn, &TitleButton::clicked, this, &TitleFrame::onButtonClicked);
    layout->addWidget(btn);
    m_buttonList << btn;

    layout->addStretch();
}

void TitleFrame::setCurrentID(TitleButton::SettingID id)
{
    for (auto button : m_buttonList) {
        if (button->id() == id) {
            button->setIsActived(true);
        }
        else {
            button->setIsActived(false);
        }
    }
}

void TitleFrame::paintEvent(QPaintEvent *e)
{
    QWidget::paintEvent(e);

    // Separator
    QRect r(width() - 1, 0, 1, height());
    QPainter p(this);
    p.fillRect(r, QColor(0, 0, 0, 25));
}

void TitleFrame::onButtonClicked(TitleButton::SettingID id)
{
    setCurrentID(id);
    emit clicked(id);
}
