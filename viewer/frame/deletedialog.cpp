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
        const int BORDER_WIDTH = 2;
        int PADDING = BORDER_WIDTH/2;
        setFixedSize(SINGLEIMAGE_SIZE);
        m_delPix =  cutSquareImage(m_delPix,
            QSize(SINGLEIMAGE_SIZE.width() - BORDER_WIDTH*2,
                  SINGLEIMAGE_SIZE.height() - BORDER_WIDTH*2));

        QPen pen(QColor(Qt::white));
        pen.setWidth(2);
        painter.setPen(pen);

        painter.drawLine(QPoint(PADDING, PADDING),
                         QPoint(width() - PADDING, PADDING));
        painter.drawLine(QPoint(width() - PADDING, PADDING),
                         QPoint(width() - PADDING, height() - PADDING));
        painter.drawLine(QPoint(width() - PADDING, height() - PADDING),
                         QPoint(PADDING, height() - PADDING));
        painter.drawLine(QPoint(PADDING, height() - PADDING),
                         QPoint(PADDING, PADDING));
        painter.drawPixmap(BORDER_WIDTH, BORDER_WIDTH, m_delPix.width(),
                           m_delPix.height(), m_delPix);
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
    setMinimumSize(380, 135);
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
