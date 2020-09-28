#ifndef GTESTVIEW_H
#define GTESTVIEW_H
#include "application.h"
#include "controller/commandline.h"
#include "service/defaultimageviewer.h"

#include "controller/configsetter.h"
#include "controller/globaleventfilter.h"
#include "controller/signalmanager.h"
#include "controller/wallpapersetter.h"
#include "controller/viewerthememanager.h"
#include "controller/dbmanager.h"
#include "controller/importer.h"
#include "utils/snifferimageformat.h"

#include "frame/bottomtoolbar.h"
#include "frame/extensionpanel.h"
#include "frame/mainwidget.h"
#include "frame/mainwindow.h"
#include "frame/renamedialog.h"
#include "frame/toptoolbar.h"
#include "module/view/viewpanel.h"
#include "module/view/lockwidget.h"
#include "module/view/navigationwidget.h"
#include "module/view/thumbnailwidget.h"
#include "module/view/contents/iconbutton.h"
#include "module/view/contents/imageinfowidget.h"
#include "module/view/contents/ttbcontent.h"
#include "module/view/contents/ttlcontent.h"
//#include "module/view/contents/ttmcontent.h"
#include "module/view/scen/imageview.h"

#include "dirwatcher/scanpathsdialog.h"
#include "dirwatcher/volumemonitor.h"
#include "dirwatcher/scanpathsitem.h"

//#include "settings/shortcut/shortcuteditor.h"
//#include "settings/shortcut/shortcutframe.h"
//#include "settings/titleframe.h"
//#include "settings/settingswindow.h"
//#include "settings/contentsframe.h"
//#include "settings/titleframe.h"

#include "widgets/pushbutton.h"
#include "widgets/returnbutton.h"
#include "widgets/printoptionspage.h"
#include "printhelper.h"
#include "blureframe.h"
//#include "scrollbar.h"

#include <QtTest/QtTest>
#include <gtest/gtest.h>
//#include "common/mainwindow.h"
#include <utils/baseutils.h>
#include <utils/imageutils.h>
#include <utils/snifferimageformat.h>
//#include "utils/imageutils_freeimage.h"
#include <unionimage.h>
//#include <DLogManager>
#include <DApplicationSettings>
using namespace UnionImage_NameSpace;
using namespace utils::base;
using namespace utils::image;

#define JPEG1 "/data/home/lmh/lmh/0916/动漫/0jll1w.jpg"
#define GIF2 "/data/home/lmh/lmh/0916/tu/1.gif"
#define JPEGPATH "/usr/share/wallpapers/deepin/marian-kroell-qElMHWePpok-unsplash.jpg"
#define SVGPATH "/data/home/lmh/lmh/0916/tu/scc.svg"
#define DDSPATH "/data/home/lmh/lmh/0916/tu/DDS.dds"
class gtestview : public ::testing::Test
{
public:
    gtestview();
    virtual void SetUp()
    {
        list.push_back(GIF2);
        list.push_back(JPEG1);

    }

    virtual void TearDown()
    {

    }
protected:
    ImageLoader* m_ImageLoader{nullptr};

    BottomToolbar*   m_bottomToolbar{nullptr};
    ExtensionPanel*  m_extensionPanel{nullptr};
    MainWidget*      m_frameMainWidget{nullptr};
    MainWindow*      m_frameMainWindow{nullptr};
    RenameDialog*    m_renameDialog{nullptr};
    TopToolbar*      m_topoolBar{nullptr};

/*module*/
    ViewPanel*       m_viewPanel{nullptr};
    LockWidget*      m_lockWidget{nullptr};
    ThumbnailWidget* m_thumbnailWidget{nullptr};
    NavigationWidget* m_navigationWidget{nullptr};

    ImageIconButton *m_ImageIconButton1{nullptr};
    ImageIconButton *m_ImageIconButton2{nullptr};

    ImageInfoWidget *m_ImageInfoWidget{nullptr};

    ImageView * m_ImageView{nullptr};


 /*dirwatcher*/
    ScanPathsDialog* m_ScanPathsDialog{nullptr};
    VolumeMonitor* m_VolumeMonitor{nullptr};
    ScanPathsItem* m_ScanPathsItem{nullptr};

//    ShortcutFrame *m_shortcutFrame;
//    ShortcutEditor *m_shortcutEditor;

//    TitleFrame *m_titleFrame;
//    SettingsWindow *m_settingsWindow;
//    ContentsFrame *m_contentsFrame;

    PushButton * m_pushbutton{nullptr};
    ReturnButton *m_returnButton{nullptr};
    PrintOptionsPage *m_printOptionspage{nullptr};
    BlurFrame *m_blurFrame{nullptr};
    CommandLine * m_command{nullptr};
    ScanPathsDialog *m_ScanDialog{nullptr};
    PrintHelper *m_Print{nullptr};

//    ScrollBar *m_scrollBar;
    QStringList list;

};

#endif // GTESTVIEW_H
