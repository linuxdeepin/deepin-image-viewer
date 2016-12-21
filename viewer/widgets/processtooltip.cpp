#include "processtooltip.h"
#include <QLabel>
#include <QTimer>
#include <QHBoxLayout>
#include <QDebug>

namespace {

const int HIDE_INTERVAL = 1500;
const int TOOLTIP_HEIGHT = 40;

}  // namespace

ProcessTooltip::ProcessTooltip(QWidget *parent)
    : BlurFrame(parent)
{
    setBorderRadius(4);
    setBorderWidth(1);
    setBorderColor(QColor(255, 255, 255, 51));

    setFixedHeight(TOOLTIP_HEIGHT);

    m_icon = new QLabel;

    m_message = new QLabel;
    m_message->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    m_message->setStyleSheet("font-size: 12px;color: white;");

    QHBoxLayout *layout = new QHBoxLayout(this);
    layout->setContentsMargins(10, 0, 20, 0);
    layout->setSpacing(12);
    layout->addWidget(m_icon);
    layout->addWidget(m_message);
}

void ProcessTooltip::showTooltip(const QString &message, bool success)
{
    if (success) {
        m_icon->setPixmap(QPixmap(":/resources/common/images/success_tick.png"));
    }
    else {
        m_icon->setPixmap(QPixmap(":/resources/common/images/failure_cross.png"));
    }
    m_message->setText(message);

    this->resize(sizeHint().width(), height());
    this->show();

    QTimer::singleShot(HIDE_INTERVAL, this, SLOT(deleteLater()));
}
