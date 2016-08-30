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

protected:
    void resizeEvent(QResizeEvent *e) Q_DECL_OVERRIDE;
    void timerEvent(QTimerEvent *e) Q_DECL_OVERRIDE;

private:
    void updateInfo();
    void updateBaseInfo(const QMap<QString, QString> &infos);
    void updateDetailsInfo(const QMap<QString, QString> &infos);
    void clearLayout(QLayout* layout);

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
