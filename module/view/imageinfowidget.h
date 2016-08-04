#ifndef IMAGEINFOWIDGET_H
#define IMAGEINFOWIDGET_H

#include <QWidget>
#include <QLabel>
#include <QScrollArea>
#include <QVector>

class QFormLayout;
class QVBoxLayout;
class ViewSeparator;

class ImageInfoWidget : public QScrollArea
{
    Q_OBJECT
public:
    explicit ImageInfoWidget(QWidget *parent = 0);
    void setImagePath(const QString &path);
//    QSize sizeHint() const override;

private:
    const QString trLabel(const char *str);
    void splitInfoStr(QString &str) const;
    void updateInfo();
    void updateBaseInfo();
    void updateDetailsInfo();
    void clearLayout(QLayout* layout);
private:
    int m_maxContentWidth;
    QString m_path;
    QFormLayout* m_exifLayout_base;
    QFormLayout* m_exifLayout_details;
    ViewSeparator* m_separator;
};

#endif // IMAGEINFOWIDGET_H
