#include "albumdelegate.h"
#include <QDateTime>
#include <QLineEdit>
#include <QPainter>
#include <QDebug>
#include <QHBoxLayout>

namespace {

const QColor BG_COLOR_NORMAL = QColor(255, 255, 255, 0);
const QColor BG_COLOR_HOVER = QColor(255, 255, 255, 26);
const QColor BG_COLOR_SELECTED = QColor(0, 188, 255, 128);

const QColor BORDER_COLOR_NORMAL = QColor(255, 255, 255, 0);
const QColor BORDER_COLOR_HOVER = QColor(255, 255, 255, 26);
const QColor BORDER_COLOR_SELECTED = QColor("#01bdff");

const int BORDER_WIDTH = 1;
const int BORDER_RADIUS = 5;

const int TITLE_FONT_SIZE = 12;
const QColor TITLE_COLOR = QColor("#ffffff");

const int THUMBNAIL_BG_MARGIN = 5;
const int THUMBNAIL_LEFT_MARGIN = 6 + 5;
const int THUMBNAIL_TOP_MARGIN = 6 + 5;
const int THUMBNAIL_RIGHT_MARGIN = 18 + 5;

const int DATELABEL_RIGHT_MARGIN = 20 + 5;
const int DATELABEL_BOTTOM_MARGIN = 2 + 8;
const double DATELABEL_FONT_SIZE = 6.7;
const QColor DATELABEL_COLOR = QColor("#000000");

}

AlbumDelegate::AlbumDelegate(QObject *parent) : QStyledItemDelegate(parent)
{

}

QWidget *AlbumDelegate::createEditor(QWidget *parent,
                                     const QStyleOptionViewItem &option,
                                     const QModelIndex &index) const
{
    Q_UNUSED(index)
    QLineEdit *lineEdit = new QLineEdit(parent);
    lineEdit->setFixedSize(option.rect.width(), 30);

    return lineEdit;
}

void AlbumDelegate::destroyEditor(QWidget *editor,
                                  const QModelIndex &index) const
{
    Q_UNUSED(index)
    editor->deleteLater();
}

void AlbumDelegate::setEditorData(QWidget *editor,
                                  const QModelIndex &index) const
{
    if (!index.isValid()) {
        return;
    }
    QList<QVariant> datas = index.model()->data(index, Qt::EditRole).toList();
    QLineEdit* lineEdit = static_cast<QLineEdit* >(editor);

    if (datas.isEmpty() || ! lineEdit) {
        return;
    }

    const QString albumName = datas[0].toString();
    lineEdit->setText(albumName);
}

void AlbumDelegate::updateEditorGeometry(QWidget *editor,
                                         const QStyleOptionViewItem &option,
                                         const QModelIndex &index) const
{
    Q_UNUSED(index)
    editor->setGeometry(option.rect);
}

void AlbumDelegate::paint(QPainter *painter,
                          const QStyleOptionViewItem &option,
                          const QModelIndex &index) const
{
    QList<QVariant> datas = index.model()->data(index, Qt::DisplayRole).toList();
    if (! datas.isEmpty()) {
        //        painter->setRenderHint(QPainter::Antialiasing);
        QRect rect = option.rect;
        rect.setSize(QSize(rect.width() - 1, rect.height() - 1));

        const QString albumName = datas[0].toString();
        const QDateTime beginTime = datas[2].toDateTime();
        const QDateTime endTime = datas[3].toDateTime();
        QPixmap thumbnail;
        thumbnail.loadFromData(datas[4].toByteArray());

        // Draw background
        drawBG(option, painter);

        // Draw compound thumbnail
        QPixmap pixmap = getCompoundPixmap(beginTime, endTime, thumbnail);
        const int pixmapSize = rect.width() - THUMBNAIL_BG_MARGIN * 2;
        QPixmap scalePixmap = pixmap.scaled(pixmapSize, pixmapSize,
                                            Qt::KeepAspectRatio,
                                            Qt::SmoothTransformation);
        painter->drawPixmap(rect.x() + THUMBNAIL_BG_MARGIN,
                            rect.y() + THUMBNAIL_BG_MARGIN,
                            pixmapSize, pixmapSize, scalePixmap);

        // Draw title
        QFont font;
        font.setPixelSize(TITLE_FONT_SIZE);
        QPen titlePen(TITLE_COLOR);
        QFontMetrics fm(font);
        painter->setFont(font);
        painter->setPen(titlePen);
        painter->drawText(rect,
                          fm.elidedText(albumName, Qt::ElideRight, rect.width()),
                          QTextOption(Qt::AlignHCenter | Qt::AlignBottom));
    }
}

QSize AlbumDelegate::sizeHint(const QStyleOptionViewItem &option,
                              const QModelIndex &index) const
{
    Q_UNUSED(option)
    return index.model()->data(index, Qt::SizeHintRole).toSize();
}

void AlbumDelegate::drawBG(const QStyleOptionViewItem &option,
                           QPainter *painter) const
{
    QRect rect = option.rect;
    rect.setSize(QSize(rect.width() - 1, rect.height() - 1));

    QColor bgColor;
    QColor borderColor;
    if (option.state & QStyle::State_MouseOver) {
        bgColor = BG_COLOR_HOVER;
        borderColor = BORDER_COLOR_HOVER;
    }
    else if (option.state & QStyle::State_Selected) {
        bgColor = BG_COLOR_SELECTED;
        borderColor = BORDER_COLOR_SELECTED;
    }
    else {
        bgColor = BG_COLOR_NORMAL;
        borderColor = BORDER_COLOR_NORMAL;
    }
    QPainterPath bgPath;
    bgPath.addRoundedRect(rect.x(), rect.y(), rect.width(), rect.height(),
                          BORDER_RADIUS, BORDER_RADIUS);
    painter->fillPath(bgPath, QBrush(bgColor));
    QPen borderPen(QBrush(borderColor), BORDER_WIDTH);
    painter->setPen(borderPen);
    painter->drawPath(bgPath);
}

QPixmap AlbumDelegate::getCompoundPixmap(const QDateTime &begin,
                                         const QDateTime &end,
                                         const QPixmap &thumbnail) const
{
    QPixmap bgPixmap(":/images/resources/images/album_bg_normal.svg");
    QPainter painter(&bgPixmap);
    painter.setRenderHint(QPainter::Antialiasing);

    const int thumbnailSize = bgPixmap.width()
            - THUMBNAIL_LEFT_MARGIN - THUMBNAIL_RIGHT_MARGIN;
    QPixmap scalePixmp = thumbnail.scaled(thumbnailSize, thumbnailSize,
                                          Qt::KeepAspectRatioByExpanding,
                                          Qt::SmoothTransformation);
    painter.drawPixmap(THUMBNAIL_LEFT_MARGIN, THUMBNAIL_TOP_MARGIN,
                       thumbnailSize, thumbnailSize, scalePixmp);
    const QString beginStr = begin.toString("yyyy");
    const QString endStr = end.toString("yyyy");
    const QString dateStr = beginStr + "-" + endStr;

    QFont font;
    font.setPixelSize(DATELABEL_FONT_SIZE);
    QFontMetrics fm(font);
    QPen titlePen(DATELABEL_COLOR);
    painter.setFont(font);
    painter.setPen(titlePen);
    QRect titleRect(bgPixmap.width() - DATELABEL_RIGHT_MARGIN - fm.width(dateStr),
                    bgPixmap.height() - DATELABEL_BOTTOM_MARGIN - fm.height(),
                    fm.width(dateStr), fm.height());
    painter.drawText(titleRect, dateStr,
                     QTextOption(Qt::AlignVCenter | Qt::AlignRight));

    return bgPixmap;
}
