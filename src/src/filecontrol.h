// SPDX-FileCopyrightText: 2020 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef FILECONTROL_H
#define FILECONTROL_H

#include "configsetter.h"

#include <QObject>
#include <QFileInfo>
#include <QTimer>
#include <QImage>
#include <QImageReader>
#include <QMap>
#include <QFileSystemWatcher>

#include "imagedata/imagefilewatcher.h"

class OcrInterface;
class QProcess;

class FileControl : public QObject
{
    Q_OBJECT
public:
    explicit FileControl(QObject *parent = nullptr);
    ~FileControl();

    //获得路径下的dir路径
    Q_INVOKABLE QString getDirPath(const QString &path);

    //获得路径下的所有图片路径
    Q_INVOKABLE QStringList getDirImagePath(const QString &path);
    //是否是图片
    Q_INVOKABLE bool isImage(const QString &path);

    //设置壁纸
    Q_INVOKABLE void setWallpaper(const QString &imgPath);
    //删除文件
    Q_INVOKABLE bool deleteImagePath(const QString &path);
    //在文件目录中显示
    Q_INVOKABLE bool displayinFileManager(const QString &path);
    //复制图片
    Q_INVOKABLE void copyImage(const QString &path);
    //复制文字
    Q_INVOKABLE void copyText(const QString &str);

    Q_INVOKABLE bool isCheckOnly();                               // 判断当前进程是否为唯一看图实例
    Q_INVOKABLE bool isSupportSetWallpaper(const QString &path);  // 是否支持设置壁纸
    Q_INVOKABLE bool isCanSupportOcr(const QString &path);
    Q_INVOKABLE bool isCanRename(const QString &path);
    Q_INVOKABLE bool isCanReadable(const QString &path);
    Q_INVOKABLE bool isRotatable(const QString &path);  // 是否可以被选旋转
    Q_INVOKABLE bool isCanWrite(const QString &path);   //是否可以被写入
    Q_INVOKABLE bool isCanDelete(const QString &path);  //是否可以被删除

    //进行ocr识别
    Q_INVOKABLE void ocrImage(const QString &path, int index);

    //解析命令行
    Q_INVOKABLE QString parseCommandlineGetPath(const QString &path);

    Q_INVOKABLE bool isDynamicImage(const QString &path);       // 是否是动态图
    Q_INVOKABLE bool isNormalStaticImage(const QString &path);  // 是否是静态图
    Q_INVOKABLE bool isSvgImage(const QString &path);           // 判断是否是svg图片
    Q_INVOKABLE bool isMultiImage(const QString &path);         // 是否是多页图
    Q_INVOKABLE bool imageIsExist(const QString &path);         // 判断图片文件是否存在

    // 旋转文件
    Q_INVOKABLE bool rotateFile(const QString &path, const int &rotateAngel);
    // 旋转
    Q_INVOKABLE void slotRotatePixCurrent();
    // 判断宽高是否互换
    Q_INVOKABLE bool isReverseHeightWidth();
    // 获取当前图片旋转角度
    Q_INVOKABLE int currentAngle();

    //获取文件名
    Q_INVOKABLE QString slotGetFileName(const QString &path);
    //获取文件名及其后缀
    Q_INVOKABLE QString slotGetFileNameSuffix(const QString &path);
    //获取某项info
    Q_INVOKABLE QString slotGetInfo(const QString &key, const QString &path);
    //公共接口，获得路径
    Q_INVOKABLE QString getNamePath(const QString &oldPath, const QString &newName);
    //文件后缀
    Q_INVOKABLE QString slotFileSuffix(const QString &path, bool ret = true);

    //文件重命名
    Q_INVOKABLE bool slotFileReName(const QString &name, const QString &filepath, bool isSuffix = false);

    //设置当前图片
    Q_INVOKABLE void setCurrentImage(const QString &path);

    // 判断重命名是否显示toolTip
    Q_INVOKABLE bool isShowToolTip(const QString &oldPath, const QString &name);

    //调用打印接口
    Q_INVOKABLE void showPrintDialog(const QString &path);

    // 设置配置文件相关值
    Q_INVOKABLE QVariant getConfigValue(const QString &group, const QString &key, const QVariant &defaultValue = QVariant());
    // 获取配置文件相关值
    Q_INVOKABLE void setConfigValue(const QString &group, const QString &key, const QVariant &value);
    Q_INVOKABLE void saveSetting();

    Q_INVOKABLE int getlastWidth();
    Q_INVOKABLE int getlastHeight();
    Q_INVOKABLE void setSettingWidth(int width);
    Q_INVOKABLE void setSettingHeight(int height);

    // 设置/获取是否允许展示导航窗口
    Q_INVOKABLE void setEnableNavigation(bool b);
    Q_INVOKABLE bool isEnableNavigation();

    // 重设当前展示图片列表
    Q_INVOKABLE void resetImageFiles(const QStringList &filePaths);

    // 获取公司 Logo 图标地址
    Q_INVOKABLE QUrl getCompanyLogo();

    // 显示快捷键面板
    Q_INVOKABLE void showShortcutPanel(int windowCenterX, int windowCenterY);
    // 结束快捷键面板进程
    Q_INVOKABLE void terminateShortcutPanelProcess();

signals:
    // 打开文件，触发 OpenImageWidget.qml:openImageFile 处理，fileName需为url路径
    void openImageFile(const QString &fileName);
    // 文件被重命名
    void imageRenamed(const QUrl &oldName, const QUrl &newName);

private:
    // 生成用于快捷键面板的字符串
    QString createShortcutString();

private:
    OcrInterface *m_ocrInterface;

    QString m_shortcutString;  // 快捷键字符串，将采用懒加载模式，需要通过createShortcutString()函数使用
    QProcess *m_shortcutViewProcess;               // 快捷键面板进程
    QStringList listsupportWallPaper;              // 支持设置壁纸的图片后缀类型
    ImageFileWatcher *imageFileWatcher = nullptr;  // 图片文件变更监控

    int m_rotateAngel = 0;           // 旋转角度
    QTimer *m_tSaveImage = nullptr;  // 保存旋转图片定时器，在指定时间内只旋转一次

    QString m_currentPath;                    // 当前操作的旋转图片路径
    QMap<QString, QString> m_currentAllInfo;  // 当前图片详细信息

    LibConfigSetter *m_config;
    int m_windowWidth = 0;
    int m_windowHeight = 0;
    int m_lastSaveWidth = 0;
    int m_lastSaveHeight = 0;
    QTimer *m_tSaveSetting = nullptr;  // 保存配置信息定时器，在指定时间内只保存一次
};

#endif  // FILECONTROL_H
