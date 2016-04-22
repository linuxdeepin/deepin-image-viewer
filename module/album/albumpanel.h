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

using namespace Dtk::Widget;

class AlbumPanel : public ModulePanel
{
    Q_OBJECT
public:
    explicit AlbumPanel(QWidget *parent = 0);

    QWidget *toolbarBottomContent() Q_DECL_OVERRIDE;
    QWidget *toolbarTopLeftContent() Q_DECL_OVERRIDE;
    QWidget *toolbarTopMiddleContent() Q_DECL_OVERRIDE;
    QWidget *extensionPanelContent() Q_DECL_OVERRIDE;

signals:
    void needGotoTimelinePanel();
    void needGotoSearchPanel();

protected:
    void dropEvent(QDropEvent *event) Q_DECL_OVERRIDE;
    void dragEnterEvent(QDragEnterEvent *event) Q_DECL_OVERRIDE;

private:
    void initMainStackWidget();
    void initAlbumsView();
    void initImagesView();
    void initStyleSheet();

    void updateBottomToolbarContent();

private slots:
    void onOpenAlbum(const QString &album);

private:
    QString m_currentAlbum;
    QLabel *m_countLabel = NULL;
    DSlider *m_slider = NULL;

    AlbumsView *m_albumsView = NULL;
    ImagesView *m_imagesView = NULL;
    QStackedWidget *m_mainStackWidget = NULL;
    DatabaseManager *m_databaseManager = DatabaseManager::instance();
    SignalManager *m_signalManager = SignalManager::instance();
};

#endif // ALBUMPANEL_H
