#ifndef NAVIGATIONWIDGET_H
#define NAVIGATIONWIDGET_H

#include "controller/viewerthememanager.h"
#include <QWidget>

class NavigationWidget : public QWidget
{
    Q_OBJECT
public:
    NavigationWidget(QWidget* parent = 0);
    void setImage(const QImage& img);
    void setRectInImage(const QRect& r);
    void setAlwaysHidden(bool value);
    bool isAlwaysHidden() const;
Q_SIGNALS:
    void requestMove(int x, int y);
protected:
    void paintEvent(QPaintEvent *);
    void mousePressEvent(QMouseEvent *e);
    void mouseMoveEvent(QMouseEvent *e);
private:
    void tryMoveRect(const QPoint& p);
    void onThemeChanged(ViewerThemeManager::AppTheme theme);
    bool m_hide = false;
    qreal m_imageScale = 1.0;
    QImage m_img;
    QPixmap m_pix;
    QRect m_r;          // Image visible rect
    QRect m_mainRect;
    QRect m_originRect;

    QString m_bgImgUrl;
    QColor m_BgColor;
    QColor m_mrBgColor;
    QColor m_mrBorderColor;
    QColor m_imgRBorderColor;
};

#endif // NAVIGATIONWIDGET_H
