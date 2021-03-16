/*
* Copyright (C) 2019 ~ 2020 Deepin Technology Co., Ltd.
*
* Author: Deng jinhui<dengjinhui@uniontech.com>
*
* Maintainer: Deng jinhui <dengjinhui@uniontech.com>
*
* This program is free software: you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation, either version 3 of the License, or
* any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program. If not, see <http://www.gnu.org/licenses/>.
*/
#ifndef IMAGEANIMATION_H
#define IMAGEANIMATION_H

#include <QtGlobal>
#include <QWidget>
#include <QPropertyAnimation>
#include <QImageReader>
#include <QObject>

#define UPDATE_RATE 30
#define FACTOR_STEP 0.005f

//#define RENDER_DEVIATION 0.005f
//#define FACTOR_BEGIN 0.0f
//#define FACTOR_END 1.0f
//#define FACTOR_EXPECTATION 0.5f
//#define FACTOR_STANDARD_DEVIATION 5
//#define NORMAL_DISTRIBUTION_MAX 0.05

QT_BEGIN_NAMESPACE

class ImageAnimationPrivate;
class ImageAnimation : public QWidget
{
    Q_OBJECT
    enum AnimationType {
        FadeEffect = 0,             //图像1渐渐变淡,图像2渐渐显现
        BlindsEffect = 1,           //百叶窗效果
        FlipRightToLeft = 2,        //图像从右向左翻转
        OutsideToInside = 3,        //从外到内水平分割
        MoveLeftToRightEffect = 4,  //图像1从左至右退出可视区域,同时图像2从左至右进入可视区域
        MoveRightToLeftEffect = 5,  //图像1从左至右退出可视区域,同时图像2从左至右进入可视区域
        MoveBottomToUpEffect = 6,   //图像1从下至上退出可视区域,同时图像2从下至上进入可视区域
        MoveUpToBottomEffect = 7,   //图像1从上至下退出可视区域,同时图像2从上至下进入可视区域
        MoveBottomToLeftUpEffect = 8//图像1不动,同时图像2从右下到左上
    };

    enum PaintTarget {
        KeepStatic = -1,
        EffectPlay = 0,
        SkipToNext = 1,
        TurnBackPre = 2
    };

    enum SlideModel {
        ManualPlayModel = 0,       //手动播放下一张
        AutoPlayModel = 1,         //自动动播放下一张
    };

    enum PlayOrStatue {
        StopStatue = 0,             //停止
        PlayStatue = 1,             //播放
    };

public:
    explicit ImageAnimation(QWidget *parent = nullptr);
    ~ImageAnimation() override;

    void startSlideShow(const QString &beginPath, const QStringList &pathlist);
    void endSlider();

    void playAndNext();
    void playAndPre();
    void pauseAndNext();
    void ifPauseAndContinue();
//    void pauseAndpre();

    const QString currentPath();
    const QRect getCurScreenGeometry();

signals:
    void singleAnimationEnd();
protected:
    void paintEvent(QPaintEvent *) override;
    void setPaintTarget(PaintTarget target);
private:
    PaintTarget current_target;
    ImageAnimationPrivate *const d_ptr;
    Q_DECLARE_PRIVATE(ImageAnimation)
    Q_DISABLE_COPY(ImageAnimation)
};

QT_END_NAMESPACE
#endif // IMAGEANIMATION_H
