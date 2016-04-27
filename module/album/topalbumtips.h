#ifndef TOPALBUMTIPS_H
#define TOPALBUMTIPS_H

#include <QFrame>

class QLabel;
class QPushButton;
class TopAlbumTips : public QFrame
{
    Q_OBJECT
public:
    explicit TopAlbumTips(QWidget *parent = 0);
    void setAlbum(const QString &album);

private:
    QString m_album;
    QLabel *m_infoLabel;
    QPushButton *m_importButton;
};

#endif // TOPALBUMTIPS_H
