#ifndef IMAGEINFOWIDGET_H
#define IMAGEINFOWIDGET_H

#include <QWidget>
#include <QLabel>
#include <QScrollArea>
#include <QVector>

class QFormLayout;
class ImageInfoWidget : public QScrollArea
{
    Q_OBJECT
public:
    explicit ImageInfoWidget(QWidget *parent = 0);
    void setImagePath(const QString &path);
//    QSize sizeHint() const override;

private:
    void updateInfo();

private:
    bool m_isDetail;
    int m_maxContentWidth;
    QString m_path;
    QFormLayout *m_exifLayout;
};

#endif // IMAGEINFOWIDGET_H
