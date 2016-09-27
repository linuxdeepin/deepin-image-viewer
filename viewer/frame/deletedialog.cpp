#include "deletedialog.h"
#include "utils/imageutils.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <dthememanager.h>
#include <QDebug>
#include <QPainter>
#include <QPushButton>

DWIDGET_USE_NAMESPACE
using namespace utils::image;
const QSize DEL_BG_SIZE = QSize(64, 64);
const QSize DEL_IMG_SIZE = QSize(54, 40);
const QSize ALBUM_SIZE = QSize(70, 70);
ConverLabel::ConverLabel(const QString &imgPath, ConverStyle converStyle, QWidget *parent)
    : QLabel(parent) {
    m_converStyle = converStyle;
    m_imagePath = imgPath;
    m_delPix.load(imgPath);
}

void ConverLabel::paintEvent(QPaintEvent *e) {
    QPainter painter(this);
    int left_margin = 0, top_margin = 0;
    switch (m_converStyle) {
    case SingleImgConver: {
        setFixedSize(DEL_BG_SIZE);
        m_delPix =  cutSquareImage(m_delPix,DEL_IMG_SIZE);

        QPixmap singleImageBg;
        singleImageBg.load(":/images/resources/images/del_single_img.png");
        painter.drawPixmap(0, 0, singleImageBg.width(), singleImageBg.height(),
                           singleImageBg);
        left_margin = (DEL_BG_SIZE.width() - DEL_IMG_SIZE.width())/2;
        top_margin = (DEL_BG_SIZE.height() - DEL_IMG_SIZE.height())/2;
        painter.drawPixmap(left_margin, top_margin, m_delPix.width(),
                           m_delPix.height(), m_delPix);
        break;
    }
    case MultiImgConver:
        setFixedSize(DEL_BG_SIZE);
        m_background.load(":/images/resources/images/del_multi_img.png");
        m_delPix = cutSquareImage(m_delPix, DEL_IMG_SIZE);
        painter.drawPixmap(0, 0, width(), height(), m_background);
        left_margin = (DEL_BG_SIZE.width() - DEL_IMG_SIZE.width())/2;
        top_margin = 14;
        painter.drawPixmap(left_margin, top_margin, m_delPix.width(),
                           m_delPix.height(), m_delPix);
        break;
    case AlbumConver: {
        setFixedSize(ALBUM_SIZE);
        m_background.load(":/images/resources/images/import_dir.png");
        m_delPix = cutSquareImage(m_delPix, QSize(ALBUM_SIZE.width() - 20,
                                                  ALBUM_SIZE.height() - 20));
        painter.drawPixmap(0, 0, width(), height(), m_background);
        painter.drawPixmap(7, 6, m_delPix.width(), m_delPix.height(), m_delPix);
        break;
    }
    default:
        break;
    }
    QLabel::paintEvent(e);
}

DeleteDialog::DeleteDialog(const QStringList imgPaths, bool isAlbum,
    QWidget *parent) : DDialog(parent) {
    QPixmap albumPix, imagesPix;
    setMinimumSize(380, 145);
    albumPix.load(":/images/resources/images/import_dir.png");
    imagesPix.load(":/images/resources/images/delete_images.png");
    setModal(true);

    QWidget* w = new QWidget(this);
    QLabel* titleLabel = new QLabel(this);
    titleLabel->setWordWrap(true);
    titleLabel->setObjectName("TitleLabel");
    titleLabel->setStyleSheet("QLabel#TitleLabel{font-size: 12px;"
                              "color: white;}");

    if (isAlbum) {
        if (imgPaths.length() >= 1)
            m_iconLabel = new ConverLabel(imgPaths.at(0), ConverLabel::AlbumConver);
        else {
           QStringList emptyPath;
           emptyPath << "";
           m_iconLabel = new ConverLabel(emptyPath.at(0), ConverLabel::AlbumConver);
        }

        titleLabel->setText(tr("Are your sure to delete this album?"));
    } else if (imgPaths.length()>1) {
        m_iconLabel = new ConverLabel(imgPaths.at(0), ConverLabel::MultiImgConver);
        titleLabel->setText(QString(tr("Are you sure to delete %1 images from "
                                       "Timeline?").arg(imgPaths.length())));

    } else if (imgPaths.length() == 1) {
        m_iconLabel = new ConverLabel(imgPaths.at(0), ConverLabel::SingleImgConver);
        titleLabel->setText(tr("Are you sure to delete this image from "
                                       "Timeline?"));
    }

    QVBoxLayout* iconLayout = new QVBoxLayout;
    iconLayout->setContentsMargins(0, 0, 0, 0);
    iconLayout->setSpacing(0);
    iconLayout->addWidget(m_iconLabel);
    iconLayout->addSpacing(10);

    QVBoxLayout* titleLayout = new QVBoxLayout;
    titleLayout->setContentsMargins(0, 0, 0, 0);
    titleLayout->setSpacing(0);
    titleLayout->addSpacing(6);
    titleLayout->addWidget(titleLabel);
    titleLayout->addStretch();

    QHBoxLayout* layout = new QHBoxLayout;
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);
    layout->addLayout(iconLayout);
    layout->addSpacing(25);
    layout->addLayout(titleLayout);
    w->setLayout(layout);

    addButton(tr("Cancel"), 0);
    addButton(tr("Delete"), 1);
    insertContent(0, w);

    for (int i = 0; i < this->children().count(); i ++) {

        QPushButton *button =
            qobject_cast<QPushButton *>(this->children().at(i));
        if (button ) {
            if (!button->text().isEmpty()){
                QFont font;
                font.setPixelSize(12);
                button->setFont(font);
            }
        }
    }

    connect(this, &DeleteDialog::buttonClicked, [=](int index) {
        if (index == 0)
            this->close();
    });
}

void DeleteDialog::keyPressEvent(QKeyEvent *e)
{
    if (e->key() == Qt::Key_Escape) {
        this->close();
    }
    DDialog::keyReleaseEvent(e);
}
