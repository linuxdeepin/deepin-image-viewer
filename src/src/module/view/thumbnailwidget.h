#ifndef THUMBNAILWIDGET_H
#define THUMBNAILWIDGET_H

#include <DWidget>
#include <DLabel>
#include <DGuiApplicationHelper>
#include <QVBoxLayout>
#include <DSuggestButton>
#include <DFontSizeManager>
#include <QPaintEvent>
#include <QMouseEvent>
#include <QObject>

class QGestureEvent;
class QPinchGesture;
class QSwipeGesture;
class QPanGesture;

DWIDGET_USE_NAMESPACE
using namespace Dtk::Widget;

class ThumbnailWidget : public DWidget
{
    Q_OBJECT
public:
    explicit ThumbnailWidget(QWidget *parent = nullptr);
    ~ThumbnailWidget() override;

public slots:

    void ThemeChange(DGuiApplicationHelper::ColorType type);

    void openImageInDialog();

private:
    bool m_isDefaultThumbnail = false;
    DLabel *m_thumbnailLabel;
    QPixmap m_logo;

    QPixmap m_defaultImage;
    QColor m_inBorderColor;
    QString m_picString;
    bool m_theme;
    bool m_usb = false;
    int m_startx = 0;
    int m_maxTouchPoints = 0;
    DLabel *m_tips;
    bool m_deepMode = false;
};

#endif // THUMBNAILWIDGET_H
