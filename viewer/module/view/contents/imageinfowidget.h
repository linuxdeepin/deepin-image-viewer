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
    void updateInfo();
//    QSize sizeHint() const override;

protected:
    void resizeEvent(QResizeEvent *e) Q_DECL_OVERRIDE;
    void timerEvent(QTimerEvent *e) Q_DECL_OVERRIDE;

private:
    void clearLayout(QLayout* layout);
    const QString trLabel(const char *str);
    void updateBaseInfo(const QMap<QString, QString> &infos);
    void updateDetailsInfo(const QMap<QString, QString> &infos);

private:
    int m_updateTid = 0;
    int m_maxTitleWidth;  //For align colon
    int m_maxFieldWidth;
    QString m_path;
    QFormLayout* m_exifLayout_base;
    QFormLayout* m_exifLayout_details;
    ViewSeparator* m_separator;
};

#endif // IMAGEINFOWIDGET_H
