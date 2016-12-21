#ifndef ALBUMPANEL_H
#define ALBUMPANEL_H

#include "albumsview.h"
#include "imagesview.h"
#include "module/modulepanel.h"
#include "controller/viewerthememanager.h"

#include <QStackedWidget>
#include <QPointer>

class Slider;
class ImportFrame;
class AlbumPanel : public ModulePanel
{
    Q_OBJECT
public:
    explicit AlbumPanel(QWidget *parent = 0);

    bool isMainPanel() Q_DECL_OVERRIDE;
    QString moduleName() Q_DECL_OVERRIDE;
    QWidget *toolbarBottomContent() Q_DECL_OVERRIDE;
    QWidget *toolbarTopLeftContent() Q_DECL_OVERRIDE;
    QWidget *toolbarTopMiddleContent() Q_DECL_OVERRIDE;
    QWidget *extensionPanelContent() Q_DECL_OVERRIDE;

public slots:
    void onOpenAlbum(const QString &album);
    void onCreateAlbum(QStringList imagepaths);

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

    void onImageCountChanged(int count);
    void onInsertIntoAlbum(const DBImgInfo info);
    void showCreateDialog(QStringList imgpaths);
    void showImportDirDialog(const QString &dir);
    void updateAlbumCount();
    void updateImagesCount(bool fromDB = false);
    void updateSliderValue(bool increase);
    void onThemeChanged(ViewerThemeManager::AppTheme theme);
private:
    QString m_currentAlbum;
    QPointer<QLabel> m_countLabel;
    Slider *m_slider;

    ImportFrame *m_importFrame;
    AlbumsView *m_albumsView = NULL;
    ImagesView *m_imagesView = NULL;
    QStackedWidget *m_stackWidget = NULL;
};

#endif // ALBUMPANEL_H
