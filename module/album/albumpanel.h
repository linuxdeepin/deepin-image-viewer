#ifndef ALBUMPANEL_H
#define ALBUMPANEL_H

#include "albumsview.h"
#include "imagesview.h"
#include "module/modulepanel.h"
#include <QStackedWidget>
#include <QPointer>

class Slider;
class ImportFrame;
class AlbumPanel : public ModulePanel
{
    Q_OBJECT
public:
    explicit AlbumPanel(QWidget *parent = 0);

    QWidget *toolbarBottomContent() Q_DECL_OVERRIDE;
    QWidget *toolbarTopLeftContent() Q_DECL_OVERRIDE;
    QWidget *toolbarTopMiddleContent() Q_DECL_OVERRIDE;
    QWidget *extensionPanelContent() Q_DECL_OVERRIDE;

public slots:
    void onOpenAlbum(const QString &album);
    void onCreateAlbum();

protected:
    void showEvent(QShowEvent *e) Q_DECL_OVERRIDE;
    void dropEvent(QDropEvent *event) Q_DECL_OVERRIDE;
    void dragEnterEvent(QDragEnterEvent *event) Q_DECL_OVERRIDE;
    void keyPressEvent(QKeyEvent *e) Q_DECL_OVERRIDE;
    void mousePressEvent(QMouseEvent *e) Q_DECL_OVERRIDE;
private:
    void initConnection();
    void initMainStackWidget();
    void initAlbumsView();
    void initImagesView();
    void initStyleSheet();

    void updateImagesCount();
    void updateAlbumCount();
    void showCreateDialog();
    void showImportDirDialog(const QString &dir);
    void onImageCountChanged(int count);
    void onInsertIntoAlbum(const DatabaseManager::ImageInfo info);

private:
    QString m_currentAlbum;
    QPointer<QLabel> m_countLabel;
    Slider *m_slider;

    AlbumsView *m_albumsView = NULL;
    ImagesView *m_imagesView = NULL;
    QStackedWidget *m_stackWidget = NULL;
    bool m_adding = false;
};

#endif // ALBUMPANEL_H
