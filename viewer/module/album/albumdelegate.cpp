#include "albumdelegate.h"
#include "application.h"
#include "controller/dbmanager.h"
#include "utils/imageutils.h"
#include "utils/baseutils.h"
#include <QDateTime>
#include <QLineEdit>
#include <QPainter>
#include <QDebug>
#include <QSvgRenderer>
#include <QHBoxLayout>

namespace {
const int TITLE_FONT_SIZE = 12;
const int TITLE_EDIT_MARGIN = 20;

const int THUMBNAIL_BG_MARGIN = 8;
const int BG_TEXT_SPACING = 5;
const int TEXTRECT_MARGIN = 20;
const int ALBUMNAME_FONTSIZE = 12;
const int FONT_WEIGHT = 25;
}

AlbumDelegate::AlbumDelegate(QObject *parent)
    : QStyledItemDelegate(parent)
    , m_editingIndex(QModelIndex())
{
    onThemeChanged(dApp->viewerTheme->getCurrentTheme());
    connect(dApp->viewerTheme, &ViewerThemeManager::viewerThemeChanged, this,
            &AlbumDelegate::onThemeChanged);
}

void AlbumDelegate::onThemeChanged(ViewerThemeManager::AppTheme theme) {
    if (theme == ViewerThemeManager::Dark) {
        m_titleColor = utils::common::DARK_TITLE_COLOR;
        m_dateColor = utils::album::DARK_DATELABEL_COLOR;
        m_createAlbumNormalPic = utils::album::DARK_CREATEALBUM_NORMALPIC;
        m_createAlbumHoverPic = utils::album::DARK_CREATEALBUM_HOVERPIC;
        m_createAlbumPressPic = utils::album::DARK_CREATEALBUM_PRESSPIC;

        m_addPic = utils::album::DARK_ADDPIC;
        m_albumBgNormalPic = utils::album::DARK_ALBUM_BG_NORMALPIC;
        m_albumBgPressPic = utils::album::DARK_ALBUM_BG_PRESSPIC;
    } else {
        m_titleColor = utils::common::LIGHT_TITLE_COLOR;
        m_dateColor = utils::album::LIGHT_DATELABEL_COLOR;
        m_createAlbumNormalPic = utils::album::LIGHT_CREATEALBUM_NORMALPIC;
        m_createAlbumHoverPic = utils::album::LIGHT_CREATEALBUM_HOVERPIC;
        m_createAlbumPressPic = utils::album::LIGHT_CREATEALBUM_PRESSPIC;

        m_addPic = utils::album::LIGHT_ADDPIC;
        m_albumBgNormalPic =utils::album::LIGHT_ALBUM_BG_NORMALPIC;
        m_albumBgPressPic = utils::album::LIGHT_ALBUM_BG_PRESSPIC;
    }
}

QWidget *AlbumDelegate::createEditor(QWidget *parent,
                                     const QStyleOptionViewItem &option,
                                     const QModelIndex &index) const
{
    Q_UNUSED(index)
    Q_UNUSED(option)
    QLineEdit *lineEdit = new QLineEdit(parent);
    lineEdit->setContextMenuPolicy(Qt::PreventContextMenu);
    lineEdit->setObjectName("AlbumNameLineEdit");
    lineEdit->setMaxLength(255);
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
    if (index.isValid()) {
        delete editor;
    }
    m_editingIndex = QModelIndex();
}

void AlbumDelegate::setEditorData(QWidget *editor,
                                  const QModelIndex &index) const
{
    if (!index.isValid()) {
        return;
    }
    QList<QVariant> datas = index.model()->data(index,
                                                Qt::DisplayRole).toList();
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
        const QStringList albums =  DBManager::instance()->getAllAlbumNames();
        QString nName = lineEdit->text().trimmed();
        if (albums.indexOf(nName) != -1 || nName.isEmpty()) {
            // Name is already exit in DB, do not rename by LineEdit's text
            nName = olds.first().toString();
        }
        lineEdit->setText(nName);
        datas.append(QVariant(nName));
        for (int i = 1; i < olds.length(); i ++) {
            datas.append(olds.at(i));
        }

        // Update database
        DBManager::instance()->renameAlbum(olds.first().toString(), nName);

        model->setData(index, QVariant(datas), Qt::DisplayRole);
    }
}

void AlbumDelegate::updateEditorGeometry(QWidget *editor,
                                         const QStyleOptionViewItem &option,
                                         const QModelIndex &index) const
{
    Q_UNUSED(index)

    QRect rect = option.rect;
    QFont font;
    font.setPixelSize(TITLE_FONT_SIZE);
    QFontMetrics fm(font);
    editor->resize(rect.width() - TITLE_EDIT_MARGIN * 2, fm.height() + 6);
    editor->move(rect.x() + TITLE_EDIT_MARGIN,
                 rect.y() + rect.height() - editor->height());
}

void AlbumDelegate::paint(QPainter *painter,
                          const QStyleOptionViewItem &option,
                          const QModelIndex &index) const
{
    QList<QVariant> datas = index.model()->data(index,
                                                Qt::DisplayRole).toList();

    QRect rect = option.rect;
    const int pixmapSize = rect.width() - THUMBNAIL_BG_MARGIN * 2;

    if (! datas.isEmpty()) {
        // Draw compound thumbnail
        QPixmap pixmap = getCompoundPixmap(option, index);
        QPixmap scalePixmap = pixmap.scaled(pixmapSize, pixmapSize,
                                            Qt::KeepAspectRatio,
                                            Qt::SmoothTransformation);
        painter->drawPixmap(rect.x() + THUMBNAIL_BG_MARGIN,
                            rect.y() + THUMBNAIL_BG_MARGIN,
                            pixmapSize, pixmapSize, scalePixmap);

        // Draw title
        drawTitle(option, index, painter);
    }
    else {
        QString createIcon = m_createAlbumNormalPic;
        if (option.state & QStyle::State_Selected) {
            createIcon = m_createAlbumPressPic;
        } else if ((option.state & QStyle::State_MouseOver)) {
            createIcon = m_createAlbumHoverPic;
        }

        QPixmap cip = QPixmap(createIcon).scaled(pixmapSize, pixmapSize);
        painter->drawPixmap(rect.x() + THUMBNAIL_BG_MARGIN,
                            rect.y() + THUMBNAIL_BG_MARGIN,
                            pixmapSize, pixmapSize, cip);
        QPixmap plus = QPixmap(m_addPic);
        int plusX =  (cip.width() -plus.width()) / 2;
        int plusY = (cip.height() -plus.height()) / 2;
        painter->drawPixmap(rect.x() + THUMBNAIL_BG_MARGIN + plusX,rect.y() +
            THUMBNAIL_BG_MARGIN + plusY, plus.width(), plus.height(), plus);
    }
}

QSize AlbumDelegate::sizeHint(const QStyleOptionViewItem &option,
                              const QModelIndex &index) const
{
    Q_UNUSED(option)
    return index.model()->data(index, Qt::SizeHintRole).toSize();
}

//void AlbumDelegate::drawBG(const QStyleOptionViewItem &option,
//                           QPainter *painter) const
//{
//    QRect rect = option.rect;
//    rect.setSize(QSize(rect.width() - 1, rect.height() - 1));

//    QColor bgColor;
//    QColor borderColor;
//    if ((option.state & QStyle::State_MouseOver) &&
//            (option.state & QStyle::State_Selected) == 0) {
//        bgColor = BG_COLOR_HOVER;
//        borderColor = BORDER_COLOR_HOVER;
//    }
//    else if (option.state & QStyle::State_Selected) {
//        bgColor = BG_COLOR_SELECTED;
//        borderColor = BORDER_COLOR_SELECTED;
//    }
//    else {
//        bgColor = BG_COLOR_NORMAL;
//        borderColor = BORDER_COLOR_NORMAL;
//    }
//    QPainterPath bgPath;
//    bgPath.addRoundedRect(rect.x(), rect.y(), rect.width(), rect.height(),
//                          BORDER_RADIUS, BORDER_RADIUS);
//    painter->fillPath(bgPath, QBrush(bgColor));
//    QPen borderPen(QBrush(borderColor), BORDER_WIDTH);
//    painter->setPen(borderPen);
//    painter->drawPath(bgPath);
//}

void AlbumDelegate::drawTitle(const QStyleOptionViewItem &option,
                              const QModelIndex &index,
                              QPainter *painter) const
{
    if (m_editingIndex != index) {
        QRect rect = option.rect;
        QFont font;
        font.setPixelSize(TITLE_FONT_SIZE);
        QPen titlePen(m_titleColor);

        QList<QVariant> datas = index.model()->data(index,
                                                    Qt::DisplayRole).toList();
        QString albumName = datas[0].toString();
        if (albumName == "Recent imported") {
            albumName = tr("Recent imported");
        }
        else if (albumName == "My favorite") {
            albumName = tr("My favorite");
        }
        painter->setPen(titlePen);
        painter->setRenderHint(QPainter::Antialiasing);

        QTextOption albumNameOption;
        albumNameOption.setAlignment(Qt::AlignHCenter | Qt::AlignTop);
        albumNameOption.setWrapMode(QTextOption::WordWrap);
        QFont albumNameFont(painter->font());
        albumNameFont.setPixelSize(ALBUMNAME_FONTSIZE);
        albumNameFont.setWeight(FONT_WEIGHT);
        const QFontMetrics fm(albumNameFont);
        QSize ts(qMin(fm.width(albumName) + TEXTRECT_MARGIN, rect.width()),
                 fm.height() + 2);

        const int TextTotalHeight = rect.height()*0.81;
        const QRectF albumNameRect = QRect(rect.x(), rect.y() + TextTotalHeight
            + BG_TEXT_SPACING, rect.width(), rect.height() - TextTotalHeight);
        QRect titleRect(rect.x() + (rect.width() - ts.width()) / 2,
            albumNameRect.y()+ (albumNameRect.height() - ts.
            height()) / 2, ts.width(), ts.height());

        // Draw albumName background
        if (option.state & QStyle::State_Selected && m_editingIndex != index) {
            QPainterPath pp;
            pp.addRoundedRect(titleRect, 4, 4);
            painter->fillPath(pp, QBrush(utils::common::TITLE_SELECTED_COLOR));
        }


        painter->setFont(albumNameFont);
        painter->drawText(albumNameRect, fm.elidedText(albumName,
            Qt::ElideMiddle, rect.width()), QTextOption(Qt::AlignCenter));
    }
}

QPixmap AlbumDelegate::getCompoundPixmap(const QStyleOptionViewItem &option,
                                         const QModelIndex &index) const
{
    QList<QVariant> datas = index.model()->data(index,
                                                Qt::DisplayRole).toList();
    const QString albumName = datas[0].toString();
    const QDateTime beginTime = datas[2].toDateTime();
    const QDateTime endTime = datas[3].toDateTime();
    QPixmap thumbnail;
    thumbnail.loadFromData(datas[4].toByteArray());


    // Render background
    QSize bgSize;
    bgSize.setWidth(option.rect.width() - THUMBNAIL_BG_MARGIN * 2);
    bgSize.setHeight(bgSize.width());
    QString bgFilePath = m_albumBgNormalPic;
    if ((option.state & QStyle::State_MouseOver) &&
            (option.state & QStyle::State_Selected) == 0) {
        bgFilePath = m_albumBgNormalPic;
    }
    else if (option.state & QStyle::State_Selected && m_editingIndex != index) {
        bgFilePath = m_albumBgPressPic;
    }
    QPixmap bgPixmap = QPixmap(bgFilePath).scaled(bgSize,
                                                  Qt::KeepAspectRatio,
                                                  Qt::SmoothTransformation);
    QPainter painter(&bgPixmap);
    painter.setRenderHint(QPainter::Antialiasing);

    // Draw thumbnail in bg
    const QRect tRect = thumbnailRect(bgSize);
    if (!thumbnail.isNull()) {
        QPixmap scalePixmp = utils::image::cutSquareImage(thumbnail, tRect.size());
        painter.drawPixmap(tRect, scalePixmp);

        // Draw album cover's out shadow
        const QRect outBorder = QRect(tRect.x() - 1, tRect.y() - 1,
                                      tRect.width() + 2, tRect.height() + 2);
        painter.setPen(QColor(0, 0, 0, 25));
        painter.drawRect(outBorder);
    }

    // Draw special mark
    int markSize = tRect.width() * 0.5;
    if (albumName == "Recent imported") {
        QPixmap p = QPixmap(":/resources/dark/images/"
                    "album_recent_imported.png").scaled(markSize, markSize,
            Qt::KeepAspectRatio, Qt::SmoothTransformation);
        painter.drawPixmap(tRect.x() + (tRect.width() - markSize) / 2,
                           tRect.y() + (tRect.height() - markSize) / 2,
                           markSize, markSize, p);
    }
    else if (albumName == "My favorite") {
        QPixmap p = QPixmap(":/resources/dark/images/album_favorites.png")
                .scaled(markSize, markSize,
                        Qt::KeepAspectRatio, Qt::SmoothTransformation);
        painter.drawPixmap(tRect.x() + (tRect.width() - markSize) / 2,
                           tRect.y() + (tRect.height() - markSize) / 2,
                           markSize, markSize, p);
    }

    // Draw year label
    const QString title = yearTitle(beginTime, endTime);
    QFont font;
    font.setPixelSize(bgSize.height() * 0.068);
    font.setWeight(FONT_WEIGHT);
    QPen titlePen(m_dateColor);
    painter.setFont(font);
    painter.setPen(titlePen);
    painter.drawText(yearTitleRect(bgSize, title), title,
                     QTextOption(Qt::AlignVCenter | Qt::AlignRight));

    return bgPixmap;
}

const QRect AlbumDelegate::thumbnailRect(const QSize &bgSize) const
{
    const int s = 0.735 * bgSize.width();
    const int lm = 0.096 * bgSize.width();
    const int tm = 0.1 * bgSize.height();

    return QRect(lm, tm, s, s);
}

const QRect AlbumDelegate::yearTitleRect(const QSize &bgSize, const QString
                                         &title) const
{
    QFont font;
    font.setPixelSize(bgSize.height() * 0.068);
    QFontMetrics fm(font);
    QRect rect(bgSize.width() - bgSize.width() * 0.1857 - fm.width(title),
                    bgSize.height() - bgSize.height() * 0.08 - fm.height() + 2,
                    fm.width(title), fm.height());
    return rect;
}

const QString AlbumDelegate::yearTitle(const QDateTime &b, const QDateTime &e)
const
{
    QString beginStr;
    QString endStr;
    QString dateStr;
    if (b.isValid() && e.isValid()) {
        beginStr = b.toString("yyyy");
        endStr = e.toString("yyyy");
    }
    else if (b.isValid() && ! e.isValid()) {
        beginStr = b.toString("yyyy");
        endStr = beginStr;
    }
    else if (! b.isValid() && e.isValid()) {
        endStr = e.toString("yyyy");
        beginStr = endStr;
    }

    if (! beginStr.isEmpty())
        dateStr = beginStr + "-" + endStr;
    return dateStr;
}

void AlbumDelegate::onEditFinished()
{
    emit editingFinished(m_editingIndex);
    m_editingIndex = QModelIndex();
}

bool AlbumDelegate::isEditFinished() {
    if (! m_editingIndex.isValid())
        return true;
    else
        return false;
}
