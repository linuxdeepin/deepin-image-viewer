#ifndef IMAGEINFOWIDGET_H
#define IMAGEINFOWIDGET_H

#include <QWidget>
#include <QLabel>
#include <QVector>
#include <dtextbutton.h>
using namespace Dtk::Widget;

class ImageInfoWidget : public QWidget
{
    Q_OBJECT
public:
    explicit ImageInfoWidget(QWidget *parent = 0);
    void setImagePath(const QString& path);
signals:

public slots:

private:
    QString m_path;
    QLabel *m_name;
    QLabel *m_date;
    QLabel *m_date_modify;
    QLabel *m_pixels_x;
    QLabel *m_pixels_y;
    QLabel *m_size;

    QWidget *m_detail;
    QVector<QLabel*> m_item;
    DTextButton *m_detail_btn;
};

#endif // IMAGEINFOWIDGET_H
