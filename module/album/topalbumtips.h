#ifndef TOPALBUMTIPS_H
#define TOPALBUMTIPS_H

#include <QFrame>

class QLabel;
class QPushButton;
class QHBoxLayout;
class TopAlbumTips : public QFrame
{
    Q_OBJECT
public:
    explicit TopAlbumTips(QWidget *parent = 0);
    void setAlbum(const QString &album);
    void setLeftMargin(int v);

private:
    const QString trName(const QString &name) const;

private:
    QHBoxLayout *m_layout;
    QString m_album;
    QLabel *m_infoLabel;
    QPushButton *m_importButton;
};

#endif // TOPALBUMTIPS_H
