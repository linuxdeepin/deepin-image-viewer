#include "importframe.h"
#include "controller/importer.h"
#include "controller/databasemanager.h"
#include "controller/signalmanager.h"
#include <QDropEvent>

#include <QPushButton>
#include <QVBoxLayout>
#include <QFileDialog>
#include <QLabel>

ImportFrame::ImportFrame(QWidget *parent)
    : QWidget(parent)
{
    this->setAcceptDrops(true);

    QLabel* bgLabel = new QLabel();
    bgLabel->setPixmap(QPixmap(":/images/resources/images/no_picture.png"));

    m_importButton = new QPushButton();
    m_importButton->setObjectName("ImportFrameButton");
    connect(m_importButton, &QPushButton::clicked, this, &ImportFrame::clicked);

    m_titleLabel = new QLabel();
    m_titleLabel->setObjectName("ImportFrameTooltip");

    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);
    layout->addStretch(1);
    layout->addWidget(bgLabel, 0, Qt::AlignHCenter);
    layout->addSpacing(21);
    layout->addWidget(m_importButton, 0, Qt::AlignHCenter);
    layout->addSpacing(15);
    layout->addWidget(m_titleLabel, 0, Qt::AlignHCenter);
    layout->addStretch(1);
}

void ImportFrame::setTitle(const QString &title)
{
    m_titleLabel->setText(title);
    QFont font;
        QFontMetrics fm(font);
        int textHeight = fm.boundingRect(m_titleLabel->text()).height();
        m_titleLabel->setMinimumHeight(textHeight + 2);
}

void ImportFrame::setButtonText(const QString &text)
{
    m_importButton->setText(text);
}

const QString ImportFrame::buttonText() const
{
    return m_importButton->text();
}

