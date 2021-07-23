#ifndef IMAGEVIEWER_GLOBAL_H
#define IMAGEVIEWER_GLOBAL_H

#include <QtCore/qglobal.h>
#include <QString>
#include <QPixmap>
#include <QImage>

#if defined(IMAGEVIEWER_LIBRARY)
#  define IMAGEVIEWERSHARED_EXPORT Q_DECL_EXPORT
#else
#  define IMAGEVIEWERSHARED_EXPORT Q_DECL_IMPORT
#endif

enum ItemInfoType {
    ItemTypeBlank = 1,//空白项，最上面留空使用
    ItemTypePic,
    ItemTypeVideo,
    ItemTypeTimeLineTitle, //时间线标题
    ItemTypeImportTimeLineTitle, //已导入时间线标题
    ItemTypeMountImg //设备图片
};

struct ItemInfo {
    QString name = "";
    QString path = "";
    QString md5Hash = "";
    int imgWidth = 0;
    int imgHeight = 0;
    QString remainDays = "30天";
    bool isSelected;
    ItemInfoType itemType = ItemTypePic;//类型，空白，图片，视频
    QImage image = QImage();
    QImage damagedPixmap = QImage();
    bool bNotSupportedOrDamaged = false;
    bool bNeedDelete = false;//删除时间线与已导入标题时使用

    QString date;
    QString num;

    friend bool operator== (const ItemInfo &left, const ItemInfo &right)
    {
        if (left.image == right.image)
            return true;
        return false;
    }
};

//图片展示方式
enum ImgViewerType {
    ImgViewerTypeNull = 0,//默认
    ImgViewerTypeLocal,   //本地图片浏览
    ImgViewerTypeAlbum    //相册浏览使用
};

Q_DECLARE_METATYPE(ItemInfo)
#endif // IMAGEVIEWER_GLOBAL_H
