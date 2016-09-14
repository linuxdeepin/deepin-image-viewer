#ifndef DELETEDIALOG_H
#define DELETEDIALOG_H

#include <ddialog.h>
#include <QPaintEvent>
#include <QLabel>

using namespace Dtk::Widget;
class ConverLabel : public QLabel {
    Q_OBJECT
public:
    enum ConverStyle {
        MultiImgConver,
        AlbumConver,
        SingleImgConver
    };
    ConverLabel(const QString &imgPath, ConverStyle converStyle = SingleImgConver,
                QWidget* parent = 0);
protected:
    void paintEvent(QPaintEvent* e);
private:
    ConverStyle m_converStyle = SingleImgConver;
    QPixmap m_delPix;
    QPixmap m_background;
    QString m_imagePath;
};

class DeleteDialog : public DDialog {
    Q_OBJECT
public:
    DeleteDialog(const QStringList imgPaths, bool isAlbum = false,
                 QWidget* parent = 0);
protected:
    void keyPressEvent(QKeyEvent *e);
private:
    ConverLabel* m_iconLabel;
    QPixmap m_delPix;
};

#endif // DELETEDIALOG_H
