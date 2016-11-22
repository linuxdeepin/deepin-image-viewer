#ifndef CREATEALBUMDIALOG_H
#define CREATEALBUMDIALOG_H

#include <ddialog.h>

using namespace Dtk::Widget;
class CreateAlbumDialog : public DDialog
{
    Q_OBJECT
public:
    explicit CreateAlbumDialog(QWidget *parent = 0);

    //getCreateAlbumName() will return the album's name;
    const QString getCreateAlbumName() const;
signals:
    void finishedAddAlbum();
protected:
    void keyPressEvent(QKeyEvent *e) override;

private:
    //getNewAlbumName() will return string like "Unnamed3", &etc;
    const QString getNewAlbumName() const;
    void createAlbum(const QString &newName);
    QString m_createAlbumName = "";
};

#endif // CREATEALBUMDIALOG_H
