#include "processtooltip.h"
#include <QLabel>
#include <QTimer>
#include <QHBoxLayout>
#include <QDebug>

namespace {

const int HIDE_INTERVAL = 1500;
const int TOOLTIP_HEIGHT = 40;

const QColor DARK_BORDER_COLOR = QColor(255, 255, 255, 51);
const QColor DARK_COVER_COLOR = QColor(0, 0, 0, 200);

const QColor LIGHT_BORDER_COLOR = QColor(0, 0, 0, 30);
const QColor LIGHT_COVER_COLOR = QColor(255, 255, 255, 230);
}  // namespace

ProcessTooltip::ProcessTooltip(QWidget *parent)
    : BlurFrame(parent)
{
    setBorderRadius(4);
    setBorderWidth(1);
    onThemeChanged(dApp->viewerTheme->getCurrentTheme());
    setBorderColor(m_borderColor);
    setCoverBrush(QBrush(m_coverColor));

    setFixedHeight(TOOLTIP_HEIGHT);

    m_icon = new QLabel;

    m_message = new QLabel;
    m_message->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    m_message->setStyleSheet(QString("font-size: 12px;color: %1;").arg(m_textColor));

    QHBoxLayout *layout = new QHBoxLayout(this);
    layout->setContentsMargins(10, 0, 20, 0);
    layout->setSpacing(12);
    layout->addWidget(m_icon);
    layout->addWidget(m_message);
    connect(dApp->viewerTheme, &ViewerThemeManager::viewerThemeChanged, this,
            [=](ViewerThemeManager::AppTheme theme) {
        onThemeChanged(theme);
        m_message->update();
    });
}

void ProcessTooltip::onThemeChanged(ViewerThemeManager::AppTheme theme) {
    if (theme == ViewerThemeManager::Dark) {
        m_borderColor = DARK_BORDER_COLOR;
        m_coverColor = DARK_COVER_COLOR;
        m_textColor = "white";
    } else {
        m_borderColor = LIGHT_BORDER_COLOR;
        m_coverColor = LIGHT_COVER_COLOR;
        m_textColor = "#303030";
    }
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
