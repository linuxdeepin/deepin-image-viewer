#ifndef IMAGEVIEWER_GLOBAL_H
#define IMAGEVIEWER_GLOBAL_H

#include <QtCore/qglobal.h>

#if defined(IMAGEVIEWER_LIBRARY)
#  define IMAGEVIEWERSHARED_EXPORT Q_DECL_EXPORT
#else
#  define IMAGEVIEWERSHARED_EXPORT Q_DECL_IMPORT
#endif

//图片展示方式
enum ImgViewerType {
    ImgViewerTypeNull = 0,//默认
    ImgViewerTypeLocal,   //本地图片浏览
    ImgViewerTypeAlbum    //相册浏览使用
};

#endif // IMAGEVIEWER_GLOBAL_H
