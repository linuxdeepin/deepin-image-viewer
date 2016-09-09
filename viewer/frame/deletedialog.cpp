#include "deletedialog.h"
#include "utils/imageutils.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <dthememanager.h>
#include <QDebug>
#include <QPainter>

DWIDGET_USE_NAMESPACE
using namespace utils::image;
const QSize SINGLEIMAGE_SIZE = QSize(60, 46);
const QSize MULTIIMAGES_SIZE = QSize(50, 42);
const QSize ALBUM_SIZE = QSize(70, 70);
ConverLabel::ConverLabel(const QString &imgPath, ConverStyle converStyle, QWidget *parent)
    : QLabel(parent) {
    m_converStyle = converStyle;
    m_imagePath = imgPath;
    m_delPix.load(imgPath);
}

void ConverLabel::paintEvent(QPaintEvent *e) {
    QPainter painter(this);
    switch (m_converStyle) {
    case SingleImgConver: {
        setFixedSize(SINGLEIMAGE_SIZE);
        m_delPix =  cutSquareImage(m_delPix,
            QSize(SINGLEIMAGE_SIZE.width() - 6, SINGLEIMAGE_SIZE.height() - 6));

        QRect borderRect = QRect(2, 2, SINGLEIMAGE_SIZE.width() - 4,
                                 SINGLEIMAGE_SIZE.height() - 4);
        QPen pen(QColor(Qt::white));
        pen.setWidth(2);
        painter.setPen(pen);
        painter.drawRect(borderRect);
        painter.drawPixmap(3, 3, m_delPix.width(), m_delPix.height(), m_delPix);
        break;
    }
    case MultiImgConver:
        setFixedSize(MULTIIMAGES_SIZE);
        m_background.load(":/images/resources/images/delete_images.png");
        m_delPix = cutSquareImage(m_delPix, QSize(MULTIIMAGES_SIZE.width() - 4,
                                                  MULTIIMAGES_SIZE.height() - 9));
        painter.drawPixmap(0, 0, width(), height(), m_background);
        painter.drawPixmap(2, 6, m_delPix.width(), m_delPix.height(), m_delPix);
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
    setMinimumWidth(380);
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

    QVBoxLayout* titleLayout = new QVBoxLayout;
    titleLayout->setContentsMargins(0, 0, 0, 0);
    titleLayout->setSpacing(0);
    titleLayout->addWidget(titleLabel);
    titleLayout->addStretch();

    QHBoxLayout* layout = new QHBoxLayout;
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);
    layout->addLayout(iconLayout);
    layout->addSpacing(25);
    layout->addLayout(titleLayout);
    w->setLayout(layout);

    QStringList buttons;
    buttons << tr("Cancel") << tr("Delete");

    addButtons(buttons);
    insertContent(0, w);
}
