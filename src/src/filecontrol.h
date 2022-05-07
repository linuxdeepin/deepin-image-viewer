#ifndef FILECONTROL_H
#define FILECONTROL_H

#include <QObject>
#include <QFileInfo>
#include <QTimer>
#include <QImage>
#include <QImageReader>
#include <QMap>

#include "configsetter.h"

class OcrInterface;

class FileControl : public QObject
{
    Q_OBJECT
public:
    explicit FileControl(QObject *parent = nullptr);
    ~  FileControl();

    //获得路径下的dir路径
    Q_INVOKABLE QString getDirPath(const QString &path);

    //获得路径下的所有图片路径
    Q_INVOKABLE QStringList getDirImagePath(const QString &path);

    //公共接口，删除list中的某项
    Q_INVOKABLE QStringList removeList(const QStringList &pathlist, int index);

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

    //是否可以被选旋转
    Q_INVOKABLE bool isRotatable(const QString &path);

    //是否可以被写入
    Q_INVOKABLE bool isCanWrite(const QString &path);

    //是否可以被删除
    Q_INVOKABLE bool isCanDelete(const QString &path);

    //是否是文件
    Q_INVOKABLE bool isFile(const QString &path);

    //进行ocr识别
    Q_INVOKABLE void ocrImage(const QString &path);

//    Q_INVOKABLE double fitImage(int imgWidth, int windowWidth);

    //解析命令行
    Q_INVOKABLE QString parseCommandlineGetPath(const QString &path)  ;

    //是否是动态图
    Q_INVOKABLE bool isDynamicImage(const QString &path);

    //是否是静态图
    Q_INVOKABLE bool isNormalStaticImage(const QString &path);

    //旋转文件
    Q_INVOKABLE bool rotateFile(const QString &path, const int &rotateAngel);

    //旋转
    Q_INVOKABLE void slotRotatePixCurrent();

    //获取文件名
    Q_INVOKABLE QString slotGetFileName(const QString &path);

    //获取文件名及其后缀
    Q_INVOKABLE QString slotGetFileNameSuffix(const QString &path);

    //获取某项info
    Q_INVOKABLE QString slotGetInfo(const QString &key, const QString &path);

    //文件重命名
    Q_INVOKABLE bool slotFileReName(const QString &name, const QString &filepath, bool isSuffix = false);

    //公共接口，重命名某一项
    Q_INVOKABLE QStringList renameOne(const QStringList &pathlist, const QString &oldPath, const QString &newPath);

    //公共接口，获得路径
    Q_INVOKABLE QString getNamePath(const  QString &oldPath, const QString &newName);

    //文件后缀
    Q_INVOKABLE QString slotFileSuffix(const QString &path, bool ret = true);

    //设置当前图片
    Q_INVOKABLE void setCurrentImage(const QString &path);

    //设置当前图片的宽度

    Q_INVOKABLE int getCurrentImageWidth();

    //设置当前图片的高度

    Q_INVOKABLE int getCurrentImageHeight();

    //获取当前的图片的适应图片的缩放比

    Q_INVOKABLE double getFitWindowScale(double WindowWidth = 800, double WindowHeight = 600);

    //判断重命名是否显示toolTip
    Q_INVOKABLE bool isShowToolTip(const QString &oldPath, const QString &name);

    //调用打印接口
    Q_INVOKABLE void showPrintDialog(const QString &path);

    //设置配置文件相关值
    Q_INVOKABLE QVariant getConfigValue(const QString &group, const QString &key,
                                        const QVariant &defaultValue = QVariant());

    //获取配置文件相关值
    Q_INVOKABLE void setConfigValue(const QString &group, const QString &key,
                                    const QVariant &value);

    Q_INVOKABLE int getlastWidth();

    Q_INVOKABLE int getlastHeight();

    Q_INVOKABLE void setSettingWidth(int width);

    Q_INVOKABLE void setSettingHeight(int height);

    Q_INVOKABLE void saveSetting();
    //是否支持设置壁纸
    Q_INVOKABLE bool isSupportSetWallpaper(const QString &path);

    Q_INVOKABLE bool isCheckOnly();

    Q_INVOKABLE bool isCanSupportOcr(const QString &path);

    Q_INVOKABLE bool isCanRename(const QString &path);

    Q_INVOKABLE bool isCanReadable(const QString &path);

    //判断是否是svg图片
    Q_INVOKABLE bool isSvgImage(const QString &path);

signals:
    void callSavePicDone();

public slots:

private :

    OcrInterface *m_ocrInterface{nullptr};

    QString m_currentPath;

    //旋转角度
    int m_rotateAngel = 0;

    QTimer *m_tSaveImage = nullptr;//保存旋转图片定时器

    QImage m_currentImage ;//当前图片
    QImageReader *m_currentReader = nullptr;

    QMap <QString, QString> m_currentAllInfo;

    int TITLEBAR_HEIGHT = 50;//标题栏高度

    LibConfigSetter *m_config;

    int m_windowWidth = 0;

    int m_windowHeight = 0;

    int m_lastSaveWidth = 0;

    int m_lastSaveHeight = 0;

    QStringList listsupportWallPaper;
};

#endif // FILECONTROL_H
