#ifndef THUMBNAILWIDGET_H
#define THUMBNAILWIDGET_H

#include <QLabel>
#include <QPaintEvent>
#include <QMouseEvent>

#include "controller/viewerthememanager.h"
#include "widgets/themewidget.h"

class ThumbnailWidget : public ThemeWidget {
    Q_OBJECT
public:
    ThumbnailWidget(const QString &darkFile, const QString
                    &lightFile, QWidget* parent = 0);
    ~ThumbnailWidget();
signals:
    void mouseHoverMoved();

public slots:
    void setThumbnailImage(const QPixmap thumbnail);
    bool isDefaultThumbnail();
protected:
    void paintEvent(QPaintEvent *event) Q_DECL_OVERRIDE;
    void mouseMoveEvent(QMouseEvent *event) Q_DECL_OVERRIDE;
private:
    void onThemeChanged(ViewerThemeManager::AppTheme theme);

    bool m_isDefaultThumbnail = false;
    QLabel* m_thumbnailLabel;
    QLabel* m_tips;
    QPixmap m_defaultImage;
    QColor m_inBorderColor;
};

#endif // THUMBNAILWIDGET_H
