#ifndef ALBUMCREATEDIALOG_H
#define ALBUMCREATEDIALOG_H

#include "dialog.h"

class AlbumCreateDialog : public Dialog
{
    Q_OBJECT
public:
    explicit AlbumCreateDialog(QWidget *parent = 0);

    const QString getCreateAlbumName() const;

signals:
    void albumAdded();

protected:
    void keyPressEvent(QKeyEvent *e) override;

private:
    void createAlbum(const QString &newName);
    const QString getNewAlbumName() const;

private:
    QString m_createAlbumName = "";
};

#endif // ALBUMCREATEDIALOG_H
