#include "progresswidgetstips.h"
#include "controller/importer.h"
#include "utils/baseutils.h"
#include <QDebug>
#include <QDesktopWidget>
#include <QHBoxLayout>
#include <QVBoxLayout>

const int WIDTH = 330;
const int HEIGHT = 62;
ProgressWidgetsTips::ProgressWidgetsTips(QWidget *parent)
    : QWidget(parent) {
    initUi();
    initConnect();
}

void ProgressWidgetsTips::initUi() {
    setFixedSize(WIDTH, HEIGHT);
    QFrame* backgroundFrame = new QFrame(this);
    backgroundFrame->setObjectName("ProgressDialog");
    m_cirProgress = new DCircleProgress(this);
    m_cirProgress->setFixedSize(40, 40);
    m_cirProgress->setValue(0);

    m_title = new QLabel(this);
    m_title->setObjectName("ProgressDialogTitle");

    m_tips = new QLabel(this);
    m_tips->setObjectName("ProgressDialogTip");

    int width = utils::base::stringWidth(m_tips->font(), m_tips->text());
    m_tips->setMinimumWidth(width);

    m_cancelButton = new DImageButton(this);
    m_cancelButton->setNormalPic(":/images/resources/images/window_close_normal.png");
    m_cancelButton->setHoverPic(":/images/resources/images/window_close_hover.png");
    m_cancelButton->setPressPic(":/images/resources/images/window_close_press.png");

    QVBoxLayout* cirLayout = new QVBoxLayout;
    cirLayout->addStretch(1);
    cirLayout->addWidget(m_cirProgress);
    cirLayout->addStretch(1);

    QVBoxLayout* contentLayout = new QVBoxLayout;
    contentLayout->setMargin(0);
    contentLayout->setSpacing(0);
    contentLayout->addStretch(1);
    contentLayout->addWidget(m_title);
    contentLayout->addSpacing(5);
    contentLayout->addWidget(m_tips);
    contentLayout->addStretch(1);

    QVBoxLayout* cancelLayout = new QVBoxLayout;
    cancelLayout->setMargin(0);
    cancelLayout->addStretch(1);
    cancelLayout->addWidget(m_cancelButton);
    cancelLayout->addStretch(1);

    QHBoxLayout* layout = new QHBoxLayout;
    layout->addLayout(cirLayout);
    layout->addLayout(contentLayout);
    layout->addSpacing(20);
    layout->addStretch();
    layout->addLayout(cancelLayout);
    layout->addSpacing(20);
    backgroundFrame->setLayout(layout);

    QHBoxLayout* mLayout = new QHBoxLayout;
    mLayout->setMargin(0);
    mLayout->setSpacing(0);
    mLayout->addWidget(backgroundFrame);

    setLayout(mLayout);
    initStyleSheet();
}

void ProgressWidgetsTips::initStyleSheet()
{
    QFile f(":/qss/resources/qss/ProgressDialog.qss");
    if (f.open(QIODevice::ReadOnly)) {
        setStyleSheet(f.readAll());
        f.close();
    }
    else {
        qDebug() << "Set style sheet for ProgressDailog failed";
    }
}

void ProgressWidgetsTips::initConnect() {
    connect(m_cancelButton, &DImageButton::clicked, [=](){
        emit stopProgress();
        close();
    });


    connect(this, &ProgressWidgetsTips::progressValueChanged, [=](int val){
        if (val==1) {
            close();
        } else {
            setValue(val);
        }
    });
    connect(this, &ProgressWidgetsTips::finished, this, &ProgressWidgetsTips::close);
}

void ProgressWidgetsTips::setValue(int value) {
    m_cirProgress->setValue(value);
}

void ProgressWidgetsTips::setTitle(QString title) {
    m_title->setText(title);
    update();
}

void ProgressWidgetsTips::setTips(QString tips) {
    m_tips->setText(tips);
    update();
}

void ProgressWidgetsTips::resizeEvent(QResizeEvent *event){

    QWidget::resizeEvent(event);
}

ProgressWidgetsTips::~ProgressWidgetsTips() {}
