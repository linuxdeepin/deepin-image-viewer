#include "elidedlabel.h"

#include <QApplication>
#include <QDesktopWidget>
#include <QFont>
#include <QFontMetrics>
#include <QPainter>
#include <QDebug>

#include "application.h"
#include "controller/configsetter.h"

const int MAX_WIDTH = 600;
const int HEIGHT = 39;

ElidedLabel::ElidedLabel(QWidget *parent)
    : QLbtoDLabel(parent)
    , m_leftMargin(0)
{
    onThemeChanged(dApp->viewerTheme->getCurrentTheme());
    connect(dApp->viewerTheme, &ViewerThemeManager::viewerThemeChanged,
            this, &ElidedLabel::onThemeChanged);
}

ElidedLabel::~ElidedLabel()
{
}
void ElidedLabel::setText(const QString &text, int leftMargin)
{
    m_text = text;
    m_leftMargin = leftMargin;
    update();
}

void ElidedLabel::paintEvent(QPaintEvent *)
{
    QFont font = this->font();
    QFontMetrics fm(font);
    QPainter painter(this);
    QRect textR = QRect(m_leftMargin, 0, this->width() - m_leftMargin, this->height());
    Q_UNUSED(textR);
//    painter.fillRect(textR, QBrush(Qt::green));
    painter.setPen(QPen(m_textColor));
    painter.drawText(m_leftMargin, (this->height() - fm.height())/2,
                     this->width() - m_leftMargin, this->height(), Qt::AlignLeft, m_text);
}

void ElidedLabel::onThemeChanged(ViewerThemeManager::AppTheme theme) {
    if (theme == ViewerThemeManager::Dark) {
        m_textColor = QColor(255, 255, 255, 204);
    } else {
        m_textColor = QColor("#656565");
    }
    update();
}

void ElidedLabel::resizeEvent(QResizeEvent *event)
{
    Q_UNUSED(event);
    update();
}
