#ifndef GTESTVIEW_H
#define GTESTVIEW_H
#include "application.h"
#include "controller/commandline.h"
#include "service/defaultimageviewer.h"

#include "controller/configsetter.h"
#include "controller/globaleventfilter.h"
#include "controller/signalmanager.h"

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

//#include "module/view/scen/imageview.h"




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

#include "testapi.h"
using namespace UnionImage_NameSpace;
using namespace utils::base;
using namespace utils::image;

#define JPEG1 "/usr/share/uosbrowser/product_logo_32.png"
#define GIF2 "/usr/share/deepin-app-store/web_dist/en-AU/assets/images/loading2.gif"
#define JPEGPATH "/usr/share/wallpapers/deepin/abc-123.jpg"
#define SVGPATH "/usr/share/deepin-image-viewer/icons/logo/deepin-image-viewer.svg"
#define DDSPATH "/data/home/lmh/0916/tu/DDS.dds"

#define test_main
#define test_module_view_contents
#define test_module_view_scen
#define test_module_view_z
#define test_service
#define test_utils
#define test_z_exit
class ScanPathsDialog;
class VolumeMonitor;
class ScanPathsItem;
class gtestview : public ::testing::Test
{
public:
    gtestview();
    virtual void SetUp()
    {
            list.clear();
            m_SVGPath=QApplication::applicationDirPath()+"/svg.svg";
            m_JPGPath=QApplication::applicationDirPath()+"/jpg.jpg";
            m_DDSPath=QApplication::applicationDirPath()+"/dds.dds";
            m_GIFPath=QApplication::applicationDirPath()+"/gif.gif";
            m_ICOPath=QApplication::applicationDirPath()+"/ico.ico";
            m_MNGPath=QApplication::applicationDirPath()+"/mng.mng";
            m_TIFPath=QApplication::applicationDirPath()+"/tif.tif";
            m_WBMPPath=QApplication::applicationDirPath()+"/wbmp.wbmp";
            m_PNGPath=QApplication::applicationDirPath()+"/png.png";
            m_GIF2Path=QApplication::applicationDirPath()+"/gif2.gif";
            m_ErrorPicPath=QApplication::applicationDirPath()+"/errorPic.icns";
            m_TgaPath=QApplication::applicationDirPath()+"/tga.tga";
            list.push_back(m_TIFPath);
            list.push_back(m_SVGPath);
            list.push_back(m_JPGPath);
            list.push_back(m_DDSPath);
            list.push_back(m_GIFPath);
            list.push_back(m_ICOPath);
            list.push_back(m_MNGPath);
            list.push_back(m_WBMPPath);
            list.push_back(m_PNGPath);
            list.push_back(m_GIF2Path);
            list.push_back(m_ErrorPicPath);
            list.push_back(m_TgaPath);


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

    QString m_SVGPath{SVGPATH};
    QString m_JPGPath{JPEGPATH};
    QString m_DDSPath{DDSPATH};
    QString m_GIFPath{GIF2};
    QString m_ICOPath;
    QString m_MNGPath;
    QString m_TIFPath;
    QString m_WBMPPath;
    QString m_PNGPath;
    QString m_GIF2Path;
    QString m_ErrorPicPath;
    QString m_TgaPath;

};

#endif // GTESTVIEW_H
