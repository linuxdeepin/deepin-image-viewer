#ifndef IMAGEBUTTON_H
#define IMAGEBUTTON_H

#include <QEvent>
#include <QLabel>
#include <dimagebutton.h>

using namespace Dtk::Widget;

class QLabel;
class ImageButton : public DImageButton
{
    Q_OBJECT
    Q_PROPERTY(QString disablePic READ getDisablePic WRITE setDisablePic
               DESIGNABLE true)
public:
    explicit ImageButton(QWidget *parent = 0);
    explicit ImageButton(const QString & normalPic, const QString & hoverPic,
                         const QString & pressPic, const QString &disablePic,
                         QWidget *parent = 0);

    void setDisablePic(const QString &path);
    void setDisabled(bool d);

    void setTooltipVisible(bool visible);
    bool tooltipVisible();

    inline const QString getDisablePic() const { return m_disablePic_; }
signals:
    void mouseLeave();

protected:
    void enterEvent(QEvent *e) Q_DECL_OVERRIDE;
    bool event(QEvent *e) Q_DECL_OVERRIDE;

private:
    void initStyleSheet();
    void showTooltip(const QPoint &gPos);

private:
    bool m_tooltipVisiable;
    QString m_disablePic_;
};

#endif // IMAGEBUTTON_H
