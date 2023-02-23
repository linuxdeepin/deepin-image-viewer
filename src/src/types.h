// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef TYPES_H
#define TYPES_H

#include <QObject>

class Types : public QObject
{
    Q_OBJECT
    Q_ENUMS(ItemRole)
    Q_ENUMS(ImageType)

public:
    explicit Types(QObject *parent = nullptr);
    ~Types() override;

    enum ItemRole {
        ImageUrlRole = Qt::UserRole + 1,  ///< 图片路径
        FrameIndexRole,                   ///< 图片帧索引
        ImageAngleRole,                   ///< 图片旋转角度
    };

    /**
       @brief 图片文件类型
     */
    enum ImageType {
        NullImage,     ///< 无图片信息，文件为空
        NormalImage,   ///< 静态图片
        DynamicImage,  ///< 动态图
        SvgImage,      ///< SVG图片
        MultiImage,    ///< 多页图
        DamagedImage,  ///< 损坏图片
    };
};

#endif  // TYPES_H
