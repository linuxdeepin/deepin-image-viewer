#include "RationalColorTransform.h"

namespace filter2d {

QRgb RationalPolynomialColorTransform1::operator ()(QRgb c)
{
    if (mDirtyC) {
        mC = colorMatrix();
        mDirtyC = isPointWise();
    }
    QVector4D C = mC*QVector4D(qRed(c), qGreen(c), qBlue(c), 255);
    postProcess1(C, c);
    if (clampMode() == EnsurePositive) {
        for (int i = 0; i < 3; ++i)
            C[i] = qAbs(C[i]);
    }
    return qRgba(qBound<int>(0, 255, C.x()),
                 qBound<int>(0, 255, C.y()),
                 qBound<int>(0, 255, C.z()),
                 qAlpha(c));
}

void RationalPolynomialColorTransform2::postProcess1(QVector4D &pass1, QRgb orig)
{
    const QRgb c = orig;
    const QVector4D C = colorMatrix1()*QVector4D(qRed(c), qGreen(c), qBlue(c), 255);
    pass1 *= C;
    postProcess2(pass1, orig);
}

void RationalFractionalColorTransform1_1::postProcess1(QVector4D &pass1, QRgb orig)
{
    const QRgb c = orig;
    const QVector4D C = colorMatrix_1()*QVector4D(qRed(c), qGreen(c), qBlue(c), 255);
    pass1 /= C;
    postProcess_2(pass1, orig);
}

} //namespace filter2d
