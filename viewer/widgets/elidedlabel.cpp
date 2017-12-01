#include "elidedlabel.h"

#include <QApplication>
#include <QDesktopWidget>
#include <QFont>
#include <QFontMetrics>
#include <QPainter>
#include <QDebug>

#include "controller/configsetter.h"

const int MAX_WIDTH = 600;
const int HEIGHT = 39;

ElidedLabel::ElidedLabel(QWidget *parent)
    : QLabel(parent)
    , m_leftMargin(0)
{
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
    QPainter painter(this);
    QRect textR = QRect(m_leftMargin, 0, this->width() - m_leftMargin, this->height());
    qDebug() << "textR" << m_leftMargin;
    painter.drawText(m_leftMargin, 0, this->width() - m_leftMargin, this->height(), Qt::AlignLeft, m_text);
}
