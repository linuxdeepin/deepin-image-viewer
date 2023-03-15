// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef GLOBALSTATUS_H
#define GLOBALSTATUS_H

#include <QObject>

class GlobalStatus : public QObject
{
    Q_OBJECT
    Q_PROPERTY(bool showRightMenu READ isShowRightMenu WRITE setShowRightMenu NOTIFY showRightMenuChanged)
    Q_PROPERTY(bool showImageInfo READ isShowImageInfo WRITE setShowImageInfo NOTIFY showImageInfoChanged)
    Q_PROPERTY(bool viewInteractive READ viewInteractive WRITE setViewInterActive NOTIFY viewInteractiveChanged)
    Q_PROPERTY(bool viewFlicking READ viewFlicking WRITE setViewFlicking NOTIFY viewFlickingChanged)
    Q_PROPERTY(int thumbnailVaildWidth READ thumbnailVaildWidth WRITE setThumbnailVaildWidth NOTIFY thumbnailVaildWidthChanged)

    // Constant properties.
    Q_PROPERTY(int minHeight READ minHeight CONSTANT)
    Q_PROPERTY(int minWidth READ minWidth CONSTANT)
    Q_PROPERTY(int minHideHeight READ minHideHeight CONSTANT)
    Q_PROPERTY(int floatMargin READ floatMargin CONSTANT)
    Q_PROPERTY(int titleHeight READ titleHeight CONSTANT)
    Q_PROPERTY(int thumbnailViewHeight READ thumbnailViewHeight CONSTANT)
    Q_PROPERTY(int showBottomY READ showBottomY CONSTANT)
    Q_PROPERTY(int switchImageHotspotWidth READ switchImageHotspotWidth CONSTANT)
    Q_PROPERTY(int actionMargin READ actionMargin CONSTANT)
    Q_PROPERTY(int rightMenuItemHeight READ rightMenuItemHeight CONSTANT)

public:
    explicit GlobalStatus(QObject *parent = nullptr);
    ~GlobalStatus() override;

    bool isShowRightMenu() const;
    void setShowRightMenu(bool b);
    Q_SIGNAL void showRightMenuChanged();

    bool isShowImageInfo() const;
    void setShowImageInfo(bool b);
    Q_SIGNAL void showImageInfoChanged();

    bool viewInteractive() const;
    void setViewInterActive(bool b);
    Q_SIGNAL void viewInteractiveChanged();

    bool viewFlicking() const;
    void setViewFlicking(bool b);
    Q_SIGNAL void viewFlickingChanged();

    void setThumbnailVaildWidth(int width);
    int thumbnailVaildWidth() const;
    Q_SIGNAL void thumbnailVaildWidthChanged();

    // Constant properties.
    int minHeight() const;
    int minWidth() const;
    int minHideHeight() const;
    int floatMargin() const;
    int titleHeight() const;
    int thumbnailViewHeight() const;
    int showBottomY() const;
    int switchImageHotspotWidth() const;
    int actionMargin() const;
    int rightMenuItemHeight() const;

private:
    bool showRightMenuDialog = false;
    bool showImageInfoDialog = false;
    bool storeViewInteractive = true;
    bool storeviewFlicking = false;
    int storeThumbnailVaildWidth = 0;  ///< 缩略图列表允许的宽度
};

#endif  // GLOBALSTATUS_H
