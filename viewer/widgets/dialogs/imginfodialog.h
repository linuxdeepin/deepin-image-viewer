#ifndef IMGINFODIALOG_H
#define IMGINFODIALOG_H

#include <ddialog.h>
#include <DMainWindow>
#include <DBlurEffectWidget>
DWIDGET_USE_NAMESPACE

class QVBoxLayout;
class ImgInfoDialog : public DMainWindow
{
    Q_OBJECT
public:
    explicit ImgInfoDialog(const QString &path, QWidget *parent = 0);

signals:
    void closed();

protected:
    void hideEvent(QHideEvent *e) Q_DECL_OVERRIDE;
    void keyPressEvent(QKeyEvent *e) Q_DECL_OVERRIDE;

private:
    void initThumbnail(const QString &path);
    void initSeparator();
    void initInfos(const QString &path);
    void initCloseButton();

private:
    QVBoxLayout *m_layout;
};

#endif // IMGINFODIALOG_H
