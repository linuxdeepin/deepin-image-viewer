#include "extensionpanel.h"
#include "application.h"
#include "controller/signalmanager.h"
#include "darrowbutton.h"
#include <QPainter>

using namespace Dtk::Widget;

namespace {

//const int CONTROL_BUTTON_WIDTH = 20;
//const int CONTROL_BUTTON_HEIGHT = 60;
//const int CONTROL_BUTTON_CUBIC_LENGTH = 30;
const int EXTENSION_PANEL_WIDTH = 240;
const int EXTENSION_PANEL_MAX_WIDTH = 340;

}  // namespace

ExtensionPanel::ExtensionPanel(QWidget *parent, QWidget *source)
    : BlureFrame(parent, source)
{
    setCoverBrush(QBrush(QColor(0, 0, 0, 100)));
    setMaximumWidth(EXTENSION_PANEL_MAX_WIDTH);
    m_contentLayout = new QHBoxLayout(this);
    m_contentLayout->setContentsMargins(0, 0, 0, 0);
    m_contentLayout->setSpacing(0);

//    DArrowButton *hideButton = new DArrowButton();
//    hideButton->setFixedSize(CONTROL_BUTTON_WIDTH, CONTROL_BUTTON_WIDTH);
//    hideButton->setArrowDirection(DArrowButton::ArrowLeft);
//    connect(hideButton, &DArrowButton::mouseRelease, [=] {
//        emit dApp->signalM->hideExtensionPanel();
//    });

//    QHBoxLayout *mainLayout = new QHBoxLayout(this);
//    mainLayout->setContentsMargins(0, 0, 0, 0);
//    mainLayout->setSpacing(0);

//    mainLayout->addLayout(m_contentLayout);
//    mainLayout->addWidget(hideButton);
//    mainLayout->addSpacing(5);
}

void ExtensionPanel::setContent(QWidget *content)
{
    if (content) {
        QLayoutItem *child;
        if ((child = m_contentLayout->takeAt(0)) != 0) {
            if (child->widget())
                child->widget()->deleteLater();
            delete child;
        }

        m_content = content;
        updateRectWithContent();
        m_contentLayout->addWidget(content);
    }
}

void ExtensionPanel::updateRectWithContent()
{
    if (m_content) {
        resize(qMax(m_content->sizeHint().width(), EXTENSION_PANEL_WIDTH),
               height());
    }
}

void ExtensionPanel::mouseMoveEvent(QMouseEvent *e)
{
    Q_UNUSED(e);
}

//void ExtensionPanel::paintEvent(QPaintEvent *)
//{
//    QPainter painter(this);
//    QPainterPath path;
//    path.moveTo(0, 0);//top left
//    path.lineTo(width() - CONTROL_BUTTON_WIDTH, 0);//top right
//    int cubicStep = 5;
//    //cubic 1
//    QPoint cubic1StartPoint(width() - CONTROL_BUTTON_WIDTH,
//                            (height() - CONTROL_BUTTON_HEIGHT) / 2 - cubicStep);
//    QPoint cubic1EndPoint(width(),
//                          cubic1StartPoint.y() + CONTROL_BUTTON_CUBIC_LENGTH);
//    path.lineTo(cubic1StartPoint); //start point of cubicTo
//    path.cubicTo(QPoint(cubic1StartPoint.x(), cubic1EndPoint.y() - cubicStep),
//        QPoint(width(), cubic1StartPoint.y() + cubicStep), cubic1EndPoint);
//    //cubic 2
//    QPoint cubic2StartPoint(width(), cubic1EndPoint.y() + (CONTROL_BUTTON_HEIGHT
//        - CONTROL_BUTTON_CUBIC_LENGTH) / 2);
//    QPoint cubic2EndPoint(width() - CONTROL_BUTTON_WIDTH,
//                          cubic2StartPoint.y() + CONTROL_BUTTON_CUBIC_LENGTH);
//    path.lineTo(cubic2StartPoint);
//    path.cubicTo(QPoint(cubic2StartPoint.x(), cubic2EndPoint.y() - cubicStep),
//                 QPoint(cubic2EndPoint.x(), cubic2StartPoint.y() + cubicStep),
//                 cubic2EndPoint);
//    path.lineTo(width() - CONTROL_BUTTON_WIDTH, height()); // Right bottom
//    path.lineTo(0, height()); // Left bottom
//    path.lineTo(0, 0); // Back to the start point

//    painter.setRenderHint(QPainter::Antialiasing);
//    painter.setRenderHint(QPainter::SmoothPixmapTransform);
//    painter.setClipPath(path);

//    painter.drawPixmap(0, 0, width(), height(), getResultPixmap());
//    painter.fillRect(0, 0, width(), height(), QBrush(QColor(0, 0, 0, 153)));

//    QPen pen;
//    pen.setColor(QColor(255, 255, 255, 51));
//    pen.setWidth(1);
//    painter.setPen(pen);
//    painter.drawPath(path);
//    painter.end();
//}
