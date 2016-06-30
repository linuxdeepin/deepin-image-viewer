#ifndef ALBUMPANEL_H
#define ALBUMPANEL_H

#include "albumsview.h"
#include "imagesview.h"
#include "module/modulepanel.h"
#include <dslider.h>
#include <QLabel>
#include <QWidget>
#include <QVBoxLayout>
#include <QStackedWidget>

class ConfigSetter;
class Slider;
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

private:
    void initMainStackWidget();
    void initAlbumsView();
    void initImagesView();
    void initStyleSheet();

    void updateBottomToolbarContent();
    void showCreateDialog();
    void showImportDirDialog(const QString &dir);
    void onImageCountChanged(int count);
    void onInsertIntoAlbum(QString album);

private:
    QString m_currentAlbum;
    QLabel *m_countLabel = NULL;
    Slider *m_slider;

    AlbumsView *m_albumsView = NULL;
    ImagesView *m_imagesView = NULL;
    QStackedWidget *m_stackWidget = NULL;
    ConfigSetter *m_setter;
    DatabaseManager *m_dbManager;
    SignalManager *m_sManager;
};

#endif // ALBUMPANEL_H
