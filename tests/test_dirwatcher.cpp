#include "gtestview.h"
#include "accessibility/ac-desktop-define.h"
#include "application.h"
#include "controller/signalmanager.h"
#define private public
#include "viewer/dirwatcher/scanpathsdialog.h"
#include "viewer/dirwatcher/scanpathsitem.h"
#include "viewer/dirwatcher/volumemonitor.h"


TEST_F(gtestview, ScanPathsDialog_show)
{
    ScanPathsDialog::instance()->show();
    DBImgInfoList list;
    DBImgInfo info;
    info.format="png";
    info.fileName=QApplication::applicationDirPath() + "/test/jpg188.jpg";
    info.filePath=QApplication::applicationDirPath() + "/test/jpg100.jpg";
    list.push_back(info);
    dApp->signalM->imagesInserted(list);
//    dApp->signalM->imagesRemoved(list);
}
TEST_F(gtestview, ScanPathsItem_1)
{
    ScanPathsItem *a=new ScanPathsItem(QApplication::applicationDirPath() + "/test/jpg101.jpg");
    a->resize(200,200);
    a->show();
    QTest::keyClick(a, Qt::Key_Enter, Qt::ControlModifier, 50);
    QTest::mouseMove(a, QPoint(20,20), 200);
    a->hide();
}
//VolumeMonitor

TEST_F(gtestview, VolumeMonitor_1)
{
    VolumeMonitor::instance()->deviceAdded("");
    VolumeMonitor::instance()->deviceRemoved("");
    VolumeMonitor::instance()->isRunning();
    VolumeMonitor::instance()->onFileChanged();
    VolumeMonitor::instance()->stop();
    VolumeMonitor::instance()->start();
    VolumeMonitor::instance()->getMountPoint("x xx x x x x  ");
    VolumeMonitor::instance()->getMountPoint("Test");
}

TEST_F(gtestview, ScanPathsDialog_1)
{
    QString path=QApplication::applicationDirPath() + "/test/jpg101.jpg";
    ScanPathsDialog::instance()->isContainByScanPaths(path);
    ScanPathsDialog::instance()->isSubPathOfScanPaths(path);
    ScanPathsDialog::instance()->addToScanPaths(path);
    ScanPathsDialog::instance()->removePath(path);
    ScanPathsDialog::instance()->show();
    ScanPathsDialog::instance()->hide();

}



