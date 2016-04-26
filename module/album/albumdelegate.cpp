#include "albumdelegate.h"
#include "controller/databasemanager.h"
#include <QDateTime>
#include <QLineEdit>
#include <QPainter>
#include <QDebug>
#include <QSvgRenderer>
#include <QHBoxLayout>

namespace {

const QColor BG_COLOR_NORMAL = QColor(255, 255, 255, 0);
const QColor BG_COLOR_HOVER = QColor(255, 255, 255, 26);
const QColor BG_COLOR_SELECTED = QColor(255, 255, 255, 0);//QColor(0, 188, 255, 128);

const QColor BORDER_COLOR_NORMAL = QColor(255, 255, 255, 0);
const QColor BORDER_COLOR_HOVER = QColor(255, 255, 255, 26);
const QColor BORDER_COLOR_SELECTED = QColor(255, 255, 255, 0);//QColor("#01bdff");

const int BORDER_WIDTH = 1;
const int BORDER_RADIUS = 5;

const int TITLE_FONT_SIZE = 12;
const QColor TITLE_COLOR = QColor("#ffffff");
const QColor TITLE_SELECTED_COLOR = QColor("#2ca7f8");
const int TITLE_EDIT_MARGIN = 20;

const int THUMBNAIL_BG_MARGIN = 8;
const int THUMBNAIL_LEFT_MARGIN = 6 + THUMBNAIL_BG_MARGIN;
const int THUMBNAIL_TOP_MARGIN = 6 + THUMBNAIL_BG_MARGIN;
const int THUMBNAIL_RIGHT_MARGIN = 18 + THUMBNAIL_BG_MARGIN;

const int DATELABEL_RIGHT_MARGIN = 20 + 5;
const int DATELABEL_BOTTOM_MARGIN = 2 + 8;
const int DATELABEL_FONT_SIZE = 9;
const QColor DATELABEL_COLOR = QColor("#000000");

}

AlbumDelegate::AlbumDelegate(QObject *parent)
    : QStyledItemDelegate(parent), m_editingIndex(QModelIndex())
{

}

QWidget *AlbumDelegate::createEditor(QWidget *parent,
                                     const QStyleOptionViewItem &option,
                                     const QModelIndex &index) const
{
    Q_UNUSED(index)
    Q_UNUSED(option)
    QLineEdit *lineEdit = new QLineEdit(parent);
    lineEdit->setObjectName("AlbumNameLineEdit");
    QFont font;
    font.setPixelSize(TITLE_FONT_SIZE);
    lineEdit->setFont(font);
    lineEdit->setAlignment(Qt::AlignCenter);
    lineEdit->setFocus();
    connect(lineEdit, &QLineEdit::editingFinished,
            this, &AlbumDelegate::onEditFinished);

    m_editingIndex = index;
    return lineEdit;
}

void AlbumDelegate::destroyEditor(QWidget *editor,
                                  const QModelIndex &index) const
{
    Q_UNUSED(index)
    m_editingIndex = QModelIndex();
    editor->deleteLater();
}

void AlbumDelegate::setEditorData(QWidget *editor,
                                  const QModelIndex &index) const
{
    if (!index.isValid()) {
        return;
    }
    QList<QVariant> datas = index.model()->data(index, Qt::DisplayRole).toList();
    QLineEdit* lineEdit = static_cast<QLineEdit* >(editor);

    if (datas.isEmpty() || ! lineEdit) {
        return;
    }

    lineEdit->setText(datas[0].toString());
}

void AlbumDelegate::setModelData(QWidget *editor, QAbstractItemModel *model,
                                 const QModelIndex &index) const
{
    QLineEdit *lineEdit = static_cast<QLineEdit*>(editor);
    if (lineEdit) {
        QList<QVariant> olds = model->data(index, Qt::DisplayRole).toList();
        QList<QVariant> datas;
        datas.append(QVariant(lineEdit->text()));
        for (int i = 1; i < olds.length(); i ++) {
            datas.append(olds.at(i));
        }

        // Update database
        DatabaseManager::instance()->renameAlbum(olds.first().toString(),
                                                 lineEdit->text());

        model->setData(index, QVariant(datas), Qt::DisplayRole);
    }
}

void AlbumDelegate::updateEditorGeometry(QWidget *editor,
                                         const QStyleOptionViewItem &option,
                                         const QModelIndex &index) const
{
    Q_UNUSED(index)

    QFont font;
    font.setPixelSize(TITLE_FONT_SIZE);
    QFontMetrics fm(font);
    editor->resize(option.rect.width() - TITLE_EDIT_MARGIN * 2, fm.height() + 6);
    editor->move(option.rect.x() + TITLE_EDIT_MARGIN,
                 option.rect.y() + option.rect.height() - editor->height());
}

void AlbumDelegate::paint(QPainter *painter,
                          const QStyleOptionViewItem &option,
                          const QModelIndex &index) const
{
    QList<QVariant> datas = index.model()->data(index, Qt::DisplayRole).toList();
    if (! datas.isEmpty()) {
        //        painter->setRenderHint(QPainter::Antialiasing);
        QRect rect = option.rect;

//        // Draw background
//        drawBG(option, painter);

        // Draw compound thumbnail
        QPixmap pixmap = getCompoundPixmap(option, index);
        const int pixmapSize = rect.width() - THUMBNAIL_BG_MARGIN * 2;
        QPixmap scalePixmap = pixmap.scaled(pixmapSize, pixmapSize,
                                            Qt::KeepAspectRatio,
                                            Qt::SmoothTransformation);
        painter->drawPixmap(rect.x() + THUMBNAIL_BG_MARGIN,
                            rect.y() + THUMBNAIL_BG_MARGIN,
                            pixmapSize, pixmapSize, scalePixmap);

        // Draw title
        drawTitle(option, index, painter);
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
    if ((option.state & QStyle::State_MouseOver) &&
            (option.state & QStyle::State_Selected) == 0) {
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

void AlbumDelegate::drawTitle(const QStyleOptionViewItem &option,
                              const QModelIndex &index,
                              QPainter *painter) const
{
    if (m_editingIndex != index) {
        const QRect rect = option.rect;
        QFont font;
        font.setPixelSize(TITLE_FONT_SIZE);
        QPen titlePen(TITLE_COLOR);
        QFontMetrics fm(font);

        QList<QVariant> datas = index.model()->data(index, Qt::DisplayRole).toList();
        QString albumName = datas[0].toString();
        if (albumName == "Recent imported") {
            albumName = tr("Recent imported");
        }
        else if (albumName == "My favorites") {
            albumName = tr("My favorites");
        }

        QSize ts(qMin(fm.width(albumName) + 20, rect.width()), fm.height() + 2);
        QRect titleRect(rect.x() + (rect.width() - ts.width()) / 2,
                        rect.y() + rect.height() - ts.height(),
                        ts.width(), ts.height());
        painter->setRenderHint(QPainter::Antialiasing);
        // Draw title background
        if (option.state & QStyle::State_Selected && m_editingIndex != index) {
            QPainterPath pp;
            pp.addRoundedRect(titleRect, 4, 4);
            painter->fillPath(pp, QBrush(TITLE_SELECTED_COLOR));
        }

        painter->setFont(font);
        painter->setPen(titlePen);
        painter->drawText(titleRect,
                          fm.elidedText(albumName,
                                        Qt::ElideRight, option.rect.width()),
                          QTextOption(Qt::AlignCenter));
    }
}

QPixmap AlbumDelegate::getCompoundPixmap(const QStyleOptionViewItem &option,
                                         const QModelIndex &index) const
{
    QList<QVariant> datas = index.model()->data(index, Qt::DisplayRole).toList();
    const QString albumName = datas[0].toString();
    const QDateTime beginTime = datas[2].toDateTime();
    const QDateTime endTime = datas[3].toDateTime();
    QPixmap thumbnail;
    thumbnail.loadFromData(datas[4].toByteArray());
    if (thumbnail.isNull()) {
        thumbnail.load(":/images/album/resources/images/album/thumbnail_default.svg");
    }

    // Render background
    QSize bgSize;
    bgSize.setWidth(option.rect.width() - THUMBNAIL_BG_MARGIN * 2);
    bgSize.setHeight(bgSize.width());
    QString bgFilePath = ":/images/album/resources/images/album/album_bg_normal.png";
    if ((option.state & QStyle::State_MouseOver) &&
            (option.state & QStyle::State_Selected) == 0) {
        bgFilePath = ":/images/album/resources/images/album/album_bg_hover.png";
    }
    else if (option.state & QStyle::State_Selected && m_editingIndex != index) {
        bgFilePath = ":/images/album/resources/images/album/album_bg_selected.png";
    }
    QPixmap bgPixmap = QPixmap(bgFilePath).scaled(bgSize,
                                                  Qt::KeepAspectRatio,
                                                  Qt::SmoothTransformation);
    QPainter painter(&bgPixmap);
    painter.setRenderHint(QPainter::Antialiasing);

    // Draw thumbnail in bg
    const int thumbnailSize = bgSize.width()
            - THUMBNAIL_LEFT_MARGIN - THUMBNAIL_RIGHT_MARGIN;
    QPixmap scalePixmp = thumbnail.scaled(thumbnailSize, thumbnailSize,
                                          Qt::KeepAspectRatioByExpanding,
                                          Qt::SmoothTransformation);
    QRect thumbnailRect(THUMBNAIL_LEFT_MARGIN, THUMBNAIL_TOP_MARGIN,
                        thumbnailSize, thumbnailSize);
    painter.drawPixmap(thumbnailRect, scalePixmp);

    // Draw special mark
    int markSize = thumbnailSize * 0.38;
    if (albumName == "Recent imported") {
        QPixmap p = QPixmap(":/images/album/resources/images/album/album_recent_imported.png")
                .scaled(markSize, markSize,
                        Qt::KeepAspectRatio, Qt::SmoothTransformation);
        painter.drawPixmap(thumbnailRect.x() + (thumbnailSize - markSize) / 2,
                           thumbnailRect.y() + (thumbnailSize - markSize) / 2,
                           markSize, markSize, p);
    }
    else if (albumName == "My favorites") {
        QPixmap p = QPixmap(":/images/album/resources/images/album/album_favorites.png")
                .scaled(markSize, markSize,
                        Qt::KeepAspectRatio, Qt::SmoothTransformation);
        painter.drawPixmap(thumbnailRect.x() + (thumbnailSize - markSize) / 2,
                           thumbnailRect.y() + (thumbnailSize - markSize) / 2,
                           markSize, markSize, p);
    }

    // Draw year label
    const QString beginStr = beginTime.toString("yyyy");
    const QString endStr = endTime.toString("yyyy");
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

void AlbumDelegate::onEditFinished()
{
    QWidget *editor = qobject_cast<QWidget *>(sender());
    emit closeEditor(editor);
    emit editingFinished(m_editingIndex);
}
