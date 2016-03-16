#ifndef COLORTRANSFORMS_H
#define COLORTRANSFORMS_H

#include "RationalColorTransform.h"
#include <cmath>

namespace filter2d {
/*
 * complex:
 * 素描 http://blog.csdn.net/yangtrees/article/details/9115321
 */
//http://blog.csdn.net/yangtrees/article/details/9116337
//怀旧色
class ColorTransform_Sepia final : public LinearColorTransform
{
protected:
    // alpha: 5x4
    virtual QMatrix4x4 colorMatrix() const override {
        return QMatrix4x4(0.393, 0.769, 0.189, 0,
                   0.348, 0.686, 0.168, 0,
                   0.272, 0.534, 0.131, 0,
                   0, 0, 0, 1);
    }
};

//冰冻
class ColorTransform_Freezing final : public LinearColorTransform
{
public:
    ClampMode clampMode() const override {return EnsurePositive;}
protected:
    virtual QMatrix4x4 colorMatrix() const override {
        static const float k = 1.5;
        return QMatrix4x4(k, -k, -k, 0,
                   -k, k, -k, 0,
                   -k, -k, k, 0,
                   0, 0, 0, 1);
    }
};

//连环画
class ColorTransform_Comic final : public RationalPolynomialColorTransform2
{
public:
    ClampMode clampMode() const override {return EnsurePositive;}
protected:
    QMatrix4x4 colorMatrix() const override {
        return QMatrix4x4(1, 2, -1, 0,
                   1, -1, 2, 0,
                   1, -1, 2, 0,
                   0, 0, 0, 1);
    }
    QMatrix4x4 colorMatrix1() const override {
        return QMatrix4x4(1, 0, 0, 0,
                   1, 0, 0, 0,
                   0, 1, 0, 0,
                   0, 0, 0, 1)/256.0;
    }
};

//熔铸
class ColorTransform_Casting : public RationalFractionalColorTransform1_1
{
protected:
    QMatrix4x4 colorMatrix() const override { return QMatrix4x4();}
    QMatrix4x4 colorMatrix_1() const override {
        return QMatrix4x4(0, 1, 1, 0,
                   1, 0, 1, 0,
                   1, 1, 0, 0,
                   0, 0, 0, 1)/128.0;
    }
};

/// color maps
struct ColorTransform_Instagram1977 final : public ColorTransform
{
    QRgb operator()(QRgb c) override;
};

struct ColorTransform_Cold final : public ColorTransform
{
    QRgb operator()(QRgb c) override;
};

/// eq
class ColorTransform_Brightness : public virtual LinearColorTransform
{
    qreal mValue = 0;
public:
    /*!
     * \brief brightness
     * 0~1 //TODO: 0~2(multiply) or -1~1(add)
     */
    qreal brightness() const {return mValue;}
    void setBrightness(qreal value) {mValue = value;}
protected:
    // alpha: 5x4
    QMatrix4x4 colorMatrix() const override {
        const int b = brightness()*255.0; //FIXME:
        return QMatrix4x4(1, 0, 0, b,
                          0, 1, 0, b,
                          0, 0, 1, b,
                          0, 0, 0, 1);
    }
};

class ColorTransform_Contrast : public virtual LinearColorTransform {
    qreal mValue = 0;
public:
    /*!
     * \brief contrast
     * 0~1
     */
    qreal contrast() const {return mValue;}
    void setContrast(qreal value) {mValue = value;}
protected:
    // alpha: 5x4
    QMatrix4x4 colorMatrix() const override {
        const float c = contrast()+1.0;
        // Contrast (offset) R,G,B
        return QMatrix4x4(c, 0, 0, 0,
                          0, c, 0, 0,
                          0, 0, c, 0,
                          0, 0, 0, 1);
    }
};

class ColorTransForm_Saturation : public virtual LinearColorTransform
{
    qreal mValue = 0;
public:
    /*!
     * \brief saturation
     * 0~1
     */
    qreal saturation() const { return mValue;}
    void setSaturation(qreal value) {mValue = value;}
protected:
    // alpha: 5x4
    QMatrix4x4 colorMatrix() const override {
        const float wr = 0.3086f;
        const float wg = 0.6094f;
        const float wb = 0.0820f;
        float s = saturation() + 1.0f;
        return QMatrix4x4(
            (1.0f - s)*wr + s, (1.0f - s)*wg    , (1.0f - s)*wb    , 0.0f,
            (1.0f - s)*wr    , (1.0f - s)*wg + s, (1.0f - s)*wb    , 0.0f,
            (1.0f - s)*wr    , (1.0f - s)*wg    , (1.0f - s)*wb + s, 0.0f,
                         0.0f,              0.0f,              0.0f, 1.0f
        );
    }

};

class ColorTransform_Hue : public virtual LinearColorTransform
{
    qreal mValue = 0;
public:
    /*!
     * \brief hue
     * 0~1
     */
    qreal hue() const { return mValue;}
    void setHue(qreal value) {mValue = value;}
protected:
    // alpha: 5x4
    QMatrix4x4 colorMatrix() const override {
        const float n = 1.0f / sqrtf(3.0f);       // normalized hue rotation axis: sqrt(3)*(1 1 1)
        const float h = hue()*M_PI;               // hue rotation angle
        const float hc = cosf(h);
        const float hs = sinf(h);
        return QMatrix4x4(     // conversion of angle/axis representation to matrix representation
            n*n*(1.0f - hc) + hc  , n*n*(1.0f - hc) - n*hs, n*n*(1.0f - hc) + n*hs, 0.0f,
            n*n*(1.0f - hc) + n*hs, n*n*(1.0f - hc) + hc  , n*n*(1.0f - hc) - n*hs, 0.0f,
            n*n*(1.0f - hc) - n*hs, n*n*(1.0f - hc) + n*hs, n*n*(1.0f - hc) + hc  , 0.0f,
                              0.0f,                   0.0f,                   0.0f, 1.0f
        );
    }
};

class ColorTransform_EQ final : public ColorTransform_Brightness, public ColorTransform_Contrast, public ColorTransForm_Saturation, public ColorTransform_Hue {
protected:
    // alpha: 5x4
    QMatrix4x4 colorMatrix() const override {
        QMatrix4x4 m = ColorTransform_Brightness::colorMatrix()*
                ColorTransform_Contrast::colorMatrix()*
                ColorTransForm_Saturation::colorMatrix()*
                ColorTransform_Hue::colorMatrix();
        qDebug() << m;
        return m;
    }
};
} //namespace filter2d
#endif // COLORTRANSFORMS_H
