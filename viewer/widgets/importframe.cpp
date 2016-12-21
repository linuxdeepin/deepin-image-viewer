#include "importframe.h"
#include "controller/importer.h"
#include "utils/baseutils.h"
#include <QDropEvent>

#include <QPushButton>
#include <QVBoxLayout>
#include <QStyle>
#include <QFileDialog>
#include <QLabel>

ImportFrame::ImportFrame(QWidget *parent)
    : QWidget(parent)
{
    this->setAcceptDrops(true);

    setDarkTheme(true);
    m_bgLabel = new QLabel();
    m_bgLabel->setFixedSize(164, 104);
    m_bgLabel->setObjectName("ImportBgLabel");


    m_importButton = new QPushButton();
    m_importButton->setFixedSize(120, 20);
    m_importButton->setObjectName("ImportFrameButton");
    connect(m_importButton, &QPushButton::clicked, this, &ImportFrame::clicked);

    m_titleLabel = new QLabel();
    m_titleLabel->setObjectName("ImportFrameTooltip");

    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);
    layout->addStretch(1);
    layout->addWidget(m_bgLabel, 0, Qt::AlignHCenter);
    layout->addSpacing(18);
    layout->addWidget(m_importButton, 0, Qt::AlignHCenter);
    layout->addSpacing(10);
    layout->addWidget(m_titleLabel, 0, Qt::AlignHCenter);
    layout->addStretch(1);

}

void ImportFrame::setTitle(const QString &title)
{
    m_titleLabel->setText(title);

    int textHeight = utils::base::stringHeight(m_titleLabel->font(), m_titleLabel->text());
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

void ImportFrame::setDarkTheme(bool dark) {
    if (dark) {
        this->setStyleSheet(utils::base::getFileContent(
                          ":/resources/dark/qss/ImportFrame.qss"));
    } else {
        this->setStyleSheet(utils::base::getFileContent(
                          ":/resources/light/qss/ImportFrame.qss"));
    }
}
