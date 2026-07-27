// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "ut_printhelper.h"
#include "printhelper.h"

#include <DDialog>
#include <dprintpreviewwidget.h>
#include <QImage>
#include <QTemporaryFile>
#include <QFile>
#include <QFileInfo>
#include <dlfcn.h>

#include "stub.h"

DWIDGET_USE_NAMESPACE

// 桩：避免 DPrintPreviewDialog（继承自 DDialog）的 exec() 阻塞测试。
// 注意：exec 为虚函数，ADDR(DDialog, exec) 返回的是 vtable 偏移而非真实代码地址，
// 会导致 Stub::set 内部 memcpy 越界。这里通过 dlsym 解析其真实符号地址。
static int ut_printhelper_stub_exec(DDialog *)
{
    return 0;
}

static void ut_printhelper_install_exec_stub(Stub &stub)
{
    void *addr = dlsym(RTLD_DEFAULT, "_ZN3Dtk6Widget7DDialog4execEv");
    ASSERT_TRUE(addr != nullptr);
    stub.set(addr, ut_printhelper_stub_exec);
}

void ut_printhelper::SetUp() {}
void ut_printhelper::TearDown() {}

// ==================== RequestedSlot ====================

// 测试构造函数初始化成员为空
TEST_F(ut_printhelper, RequestedSlot_Construct_InitializesEmptyMembers)
{
    RequestedSlot slot;
    EXPECT_TRUE(slot.m_paths.isEmpty());
    EXPECT_TRUE(slot.m_imgs.isEmpty());
}

// 测试析构函数不崩溃
TEST_F(ut_printhelper, RequestedSlot_Destruct_DoesNotCrash)
{
    RequestedSlot *slot = new RequestedSlot();
    delete slot;
    SUCCEED();
}

// 测试 public 成员 m_paths / m_imgs 可读写
TEST_F(ut_printhelper, RequestedSlot_PublicMembers_ReadWrite)
{
    RequestedSlot slot;
    slot.m_paths << "path1" << "path2";
    QImage img(10, 10, QImage::Format_RGB32);
    slot.m_imgs << img;
    EXPECT_EQ(slot.m_paths.size(), 2);
    EXPECT_EQ(slot.m_imgs.size(), 1);
}

// paintRequestSync: m_imgs 为空时仅创建 painter 并 end，不崩溃
TEST_F(ut_printhelper, RequestedSlot_PaintRequestSync_EmptyImgs_NoCrash)
{
    RequestedSlot slot;
    DPrinter printer;
    slot.paintRequestSync(&printer);  // private slot，借助 -fno-access-control 直调
    SUCCEED();
}

// paintRequestSync: 单张有效图片，走 drawImage 分支
TEST_F(ut_printhelper, RequestedSlot_PaintRequestSync_SingleValidImage)
{
    RequestedSlot slot;
    QImage img(100, 100, QImage::Format_RGB32);
    img.fill(Qt::white);
    slot.m_imgs << img;
    DPrinter printer;
    slot.paintRequestSync(&printer);
    SUCCEED();
}

// paintRequestSync: 含 null 图片（跳过绘制分支）+ 有效图片
TEST_F(ut_printhelper, RequestedSlot_PaintRequestSync_NullImageSkipped)
{
    RequestedSlot slot;
    QImage nullImg;  // null
    QImage validImg(50, 50, QImage::Format_RGB32);
    validImg.fill(Qt::red);
    slot.m_imgs << nullImg << validImg;
    DPrinter printer;
    slot.paintRequestSync(&printer);
    SUCCEED();
}

// paintRequestSync: 多张有效图片触发 newPage 分支
TEST_F(ut_printhelper, RequestedSlot_PaintRequestSync_MultipleImages_NewPage)
{
    RequestedSlot slot;
    QImage img1(80, 80, QImage::Format_RGB32);
    img1.fill(Qt::blue);
    QImage img2(120, 60, QImage::Format_RGB32);
    img2.fill(Qt::green);
    slot.m_imgs << img1 << img2;
    DPrinter printer;
    slot.paintRequestSync(&printer);
    SUCCEED();
}

// paintRequestSync: 宽扁图片（适配宽度分支）
TEST_F(ut_printhelper, RequestedSlot_PaintRequestSync_WideImage_FitWidth)
{
    RequestedSlot slot;
    QImage wide(300, 30, QImage::Format_RGB32);
    wide.fill(Qt::yellow);
    slot.m_imgs << wide;
    DPrinter printer;
    slot.paintRequestSync(&printer);
    SUCCEED();
}

// ==================== PrintHelper ====================

// getIntance: 返回非空且单例一致
TEST_F(ut_printhelper, PrintHelper_GetIntance_ReturnsSingleton)
{
    PrintHelper *p1 = PrintHelper::getIntance();
    PrintHelper *p2 = PrintHelper::getIntance();
    EXPECT_NE(p1, nullptr);
    EXPECT_EQ(p1, p2);
}

// getIntance 后实例持有 m_re
TEST_F(ut_printhelper, PrintHelper_Instance_HasRequestedSlotMember)
{
    PrintHelper *p = PrintHelper::getIntance();
    EXPECT_NE(p->m_re, nullptr);
}

// 私有构造函数（借 -fno-access-control 直调）创建 m_re
TEST_F(ut_printhelper, PrintHelper_PrivateConstructor_CreatesRequestedSlot)
{
    PrintHelper *p = new PrintHelper();
    EXPECT_NE(p->m_re, nullptr);
    delete p;  // 析构会调用 m_re->deleteLater()，随后 QObject 销毁子对象
}

// showPrintDialog: 空路径列表，exec 被桩住，结束后成员被清空
TEST_F(ut_printhelper, PrintHelper_ShowPrintDialog_EmptyPaths_ClearsState)
{
    PrintHelper *p = PrintHelper::getIntance();
    Stub stub;
    ut_printhelper_install_exec_stub(stub);
    p->showPrintDialog(QStringList(), nullptr);
    EXPECT_TRUE(p->m_re->m_paths.isEmpty());
    EXPECT_TRUE(p->m_re->m_imgs.isEmpty());
}

// showPrintDialog: 不存在的路径，图片加载失败但不崩溃
TEST_F(ut_printhelper, PrintHelper_ShowPrintDialog_NonexistentPath_NoCrash)
{
    PrintHelper *p = PrintHelper::getIntance();
    Stub stub;
    ut_printhelper_install_exec_stub(stub);
    QStringList paths;
    paths << "/tmp/ut_printhelper_nonexistent_12345.png";
    p->showPrintDialog(paths, nullptr);
    EXPECT_TRUE(p->m_re->m_paths.isEmpty());
    EXPECT_TRUE(p->m_re->m_imgs.isEmpty());
}

// showPrintDialog: 有效图片路径，加载后走 setDocName 路径
TEST_F(ut_printhelper, PrintHelper_ShowPrintDialog_ValidImage_LoadsAndClears)
{
    // 生成临时 PNG 文件
    QString tmpPath = QString("/tmp/ut_printhelper_%1.png").arg(QCoreApplication::instance()->applicationPid());
    QImage img(50, 50, QImage::Format_RGB32);
    img.fill(Qt::blue);
    ASSERT_TRUE(img.save(tmpPath, "PNG"));

    PrintHelper *p = PrintHelper::getIntance();
    Stub stub;
    ut_printhelper_install_exec_stub(stub);
    QStringList paths;
    paths << tmpPath;
    p->showPrintDialog(paths, nullptr);
    EXPECT_TRUE(p->m_re->m_paths.isEmpty());
    EXPECT_TRUE(p->m_re->m_imgs.isEmpty());

    QFile::remove(tmpPath);
}

// showPrintDialog: 多路径混合（含一个有效）
TEST_F(ut_printhelper, PrintHelper_ShowPrintDialog_MultiplePaths_NoCrash)
{
    QString tmpPath = QString("/tmp/ut_printhelper_multi_%1.jpg").arg(QCoreApplication::instance()->applicationPid());
    QImage img(40, 60, QImage::Format_RGB32);
    img.fill(Qt::green);
    ASSERT_TRUE(img.save(tmpPath, "JPG"));

    PrintHelper *p = PrintHelper::getIntance();
    Stub stub;
    ut_printhelper_install_exec_stub(stub);
    QStringList paths;
    paths << tmpPath << "/tmp/ut_printhelper_missing.png";
    p->showPrintDialog(paths, nullptr);
    EXPECT_TRUE(p->m_re->m_paths.isEmpty());

    QFile::remove(tmpPath);
}
