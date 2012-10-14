
/*
   Copyright (c) 2003-2007 Clarence Dang <dang@kde.org>
   All rights reserved.

   Redistribution and use in source and binary forms, with or without
   modification, are permitted provided that the following conditions
   are met:

   1. Redistributions of source code must retain the above copyright
      notice, this list of conditions and the following disclaimer.
   2. Redistributions in binary form must reproduce the above copyright
      notice, this list of conditions and the following disclaimer in the
      documentation and/or other materials provided with the distribution.

   THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
   IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
   OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
   IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
   INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
   NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
   DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
   THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
   (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
   THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/


#define DEBUG_KP_PIXMAP_FX 0


#include <kpPixmapFX.h>

#include <math.h>

#include <qpainter.h>
#include <QImage>
#include <qpoint.h>
#include <qrect.h>

#include <kdebug.h>

#include <kpAbstractSelection.h>
#include <kpColor.h>
#include <kpDefs.h>

//---------------------------------------------------------------------

// public static
void kpPixmapFX::resize (QImage *destPtr, int w, int h,
                         const kpColor &backgroundColor)
{
#if DEBUG_KP_PIXMAP_FX && 1
    kDebug () << "kpPixmapFX::resize()";
#endif

    if (!destPtr)
        return;

    const int oldWidth = destPtr->width ();
    const int oldHeight = destPtr->height ();

    if (w == oldWidth && h == oldHeight)
        return;

    QImage newImage (w, h, QImage::Format_ARGB32_Premultiplied);

    // Would have new undefined areas?
    if (w > oldWidth || h > oldHeight)
        newImage.fill (backgroundColor.toQRgb ());

    // Copy over old pixmap.
    QPainter painter(&newImage);
    painter.setCompositionMode(QPainter::CompositionMode_Source);
    painter.drawImage(0, 0, *destPtr);
    painter.end();

    // Replace pixmap with new one.
    *destPtr = newImage;
}

//---------------------------------------------------------------------

// public static
QImage kpPixmapFX::resize (const QImage &image, int w, int h,
                           const kpColor &backgroundColor)
{
    QImage ret = image;
    kpPixmapFX::resize (&ret, w, h, backgroundColor);
    return ret;
}

//---------------------------------------------------------------------

// public static
void kpPixmapFX::scale (QImage *destPtr, int w, int h, bool pretty)
{
    if (!destPtr)
        return;

    *destPtr = kpPixmapFX::scale (*destPtr, w, h, pretty);
}

//---------------------------------------------------------------------

// public static
QImage kpPixmapFX::scale (const QImage &image, int w, int h, bool pretty)
{
#if DEBUG_KP_PIXMAP_FX && 0
    kDebug () << "kpPixmapFX::scale(oldRect=" << image.rect ()
               << ",w=" << w
               << ",h=" << h
               << ",pretty=" << pretty
               << ")"
               << endl;
#endif

    if (w == image.width () && h == image.height ())
        return image;

    return image.scaled(w, h, Qt::IgnoreAspectRatio,
                        pretty ? Qt::SmoothTransformation : Qt::FastTransformation);
}

//---------------------------------------------------------------------

// public static
const double kpPixmapFX::AngleInDegreesEpsilon =
    KP_RADIANS_TO_DEGREES (atan (1.0 / 10000.0))
        / (2.0/*max error allowed*/ * 2.0/*for good measure*/);


static void MatrixDebug (const QString matrixName, const QMatrix &matrix,
        int srcPixmapWidth = -1, int srcPixmapHeight = -1)
{
#if DEBUG_KP_PIXMAP_FX
    const int w = srcPixmapWidth, h = srcPixmapHeight;

    kDebug () << matrixName << "=" << matrix;
    // Sometimes this precision lets us see unexpected rounding errors.
    fprintf (stderr, "m11=%.24f m12=%.24f m21=%.24f m22=%.24f dx=%.24f dy=%.24f\n",
             matrix.m11 (), matrix.m12 (),
             matrix.m21 (), matrix.m22 (),
             matrix.dx (), matrix.dy ());
    if (w > 0 && h > 0)
    {
        kDebug () << "(0,0) ->" << matrix.map (QPoint (0, 0));
        kDebug () << "(w-1,0) ->" << matrix.map (QPoint (w - 1, 0));
        kDebug () << "(0,h-1) ->" << matrix.map (QPoint (0, h - 1));
        kDebug () << "(w-1,h-1) ->" << matrix.map (QPoint (w - 1, h - 1));
    }

#if 0
    QMatrix trueMatrix = QPixmap::trueMatrix (matrix, w, h);
    kDebug () << matrixName << "trueMatrix=" << trueMatrix;
    if (w > 0 && h > 0)
    {
        kDebug () << "(0,0) ->" << trueMatrix.map (QPoint (0, 0));
        kDebug () << "(w-1,0) ->" << trueMatrix.map (QPoint (w - 1, 0));
        kDebug () << "(0,h-1) ->" << trueMatrix.map (QPoint (0, h - 1));
        kDebug () << "(w-1,h-1) ->" << trueMatrix.map (QPoint (w - 1, h - 1));
    }
#endif

#else

    Q_UNUSED (matrixName);
    Q_UNUSED (matrix);
    Q_UNUSED (srcPixmapWidth);
    Q_UNUSED (srcPixmapHeight);

#endif  // DEBUG_KP_PIXMAP_FX
}

//---------------------------------------------------------------------

// Theoretically, this should act the same as QPixmap::trueMatrix() but
// it doesn't.  As an example, if you rotate tests/transforms.png by 90
// degrees clockwise, this returns the correct <dx> of 26 but
// QPixmap::trueMatrix() returns 27.
//
// You should use the returned matrix to map points accurately (e.g. selection
// borders).  For QPainter::drawPixmap()/drawImage() + setWorldMatrix()
// rendering accuracy, pass the returned matrix through QPixmap::trueMatrix()
// and use that.
//
// TODO: If you put the flipMatrix() of tests/transforms.png through this,
//       the output is the same as QPixmap::trueMatrix(): <dy> is one off
//       (dy=27 instead of 26).
//       SYNC: I bet this is a Qt4 bug.
static QMatrix MatrixWithZeroOrigin (const QMatrix &matrix, int width, int height)
{
#if DEBUG_KP_PIXMAP_FX
    kDebug () << "matrixWithZeroOrigin(w=" << width << ",h=" << height << ")";
    kDebug () << "\tmatrix: m11=" << matrix.m11 ()
               << "m12=" << matrix.m12 ()
               << "m21=" << matrix.m21 ()
               << "m22=" << matrix.m22 ()
               << "dx=" << matrix.dx ()
               << "dy=" << matrix.dy ();
#endif

    QRect mappedRect = matrix.mapRect (QRect (0, 0, width, height));
#if DEBUG_KP_PIXMAP_FX
    kDebug () << "\tmappedRect=" << mappedRect;
#endif

    QMatrix translatedMatrix (
        matrix.m11 (), matrix.m12 (),
        matrix.m21 (), matrix.m22 (),
        matrix.dx () - mappedRect.left (), matrix.dy () - mappedRect.top ());

#if DEBUG_KP_PIXMAP_FX
    kDebug () << "\treturning" << translatedMatrix;
    kDebug () << "(0,0) ->" << translatedMatrix.map (QPoint (0, 0));
    kDebug () << "(w-1,0) ->" << translatedMatrix.map (QPoint (width - 1, 0));
    kDebug () << "(0,h-1) ->" << translatedMatrix.map (QPoint (0, height - 1));
    kDebug () << "(w-1,h-1) ->" << translatedMatrix.map (QPoint (width - 1, height - 1));
#endif

    return translatedMatrix;
}

//---------------------------------------------------------------------

static double TrueMatrixEpsilon = 0.000001;

// An attempt to reverse tiny rounding errors introduced by QPixmap::trueMatrix()
// when skewing tests/transforms.png by 45% horizontally (with TransformPixmap()
// using a QPixmap painter, prior to the 2007-10-09 change -- did not test after
// the change).
// Unfortunately, this does not work enough to stop the rendering errors
// that follow.  But it was worth a try and might still help us given the
// sometimes excessive aliasing QPainter::draw{Pixmap,Image}() gives us, when
// QPainter::SmoothPixmapTransform is disabled.
static double TrueMatrixFixInts (double x)
{
    if (fabs (x - qRound (x)) < TrueMatrixEpsilon)
        return qRound (x);
    else
        return x;
}

//---------------------------------------------------------------------

static QMatrix TrueMatrix (const QMatrix &matrix, int srcPixmapWidth, int srcPixmapHeight)
{
    ::MatrixDebug ("TrueMatrix(): org", matrix);
    
    const QMatrix truMat = QPixmap::trueMatrix (matrix, srcPixmapWidth, srcPixmapHeight);
    ::MatrixDebug ("TrueMatrix(): passed through QPixmap::trueMatrix()", truMat);

    const QMatrix retMat (
        ::TrueMatrixFixInts (truMat.m11 ()),
        ::TrueMatrixFixInts (truMat.m12 ()),
        ::TrueMatrixFixInts (truMat.m21 ()),
        ::TrueMatrixFixInts (truMat.m22 ()),
        ::TrueMatrixFixInts (truMat.dx ()),
        ::TrueMatrixFixInts (truMat.dy ()));
    ::MatrixDebug ("TrueMatrix(): fixed ints", retMat);

    return retMat;
}

//---------------------------------------------------------------------

// Like QPixmap::transformed() but fills new areas with <backgroundColor>
// (unless <backgroundColor> is invalid) and works around internal QMatrix
// floating point -> integer oddities, that would otherwise give fatally
// incorrect results.  If you don't believe me on this latter point, compare
// QPixmap::transformed() to us using a flip matrix or a rotate-by-multiple-of-90
// matrix on tests/transforms.png -- QPixmap::transformed()'s output is 1
// pixel too high or low depending on whether the matrix is passed through
// QPixmap::trueMatrix().
//
// Use <targetWidth> and <targetHeight> to specify the intended output size
// of the pixmap.  -1 if don't care.
static QImage TransformPixmap (const QImage &pm, const QMatrix &transformMatrix_,
        const kpColor &backgroundColor,
        int targetWidth, int targetHeight)
{
    QMatrix transformMatrix = transformMatrix_;

#if DEBUG_KP_PIXMAP_FX && 1
    kDebug () << "kppixmapfx.cpp: TransformPixmap(pm.size=" << pm.size ()
               << ",targetWidth=" << targetWidth
               << ",targetHeight=" << targetHeight
               << ")"
               << endl;
#endif

    QRect newRect = transformMatrix.mapRect (pm.rect ());
#if DEBUG_KP_PIXMAP_FX && 1
    kDebug () << "\tmappedRect=" << newRect;

#endif

    QMatrix scaleMatrix;
    if (targetWidth > 0 && targetWidth != newRect.width ())
    {
    #if DEBUG_KP_PIXMAP_FX && 1
        kDebug () << "\tadjusting for targetWidth";
    #endif
        scaleMatrix.scale (double (targetWidth) / double (newRect.width ()), 1);
    }

    if (targetHeight > 0 && targetHeight != newRect.height ())
    {
    #if DEBUG_KP_PIXMAP_FX && 1
        kDebug () << "\tadjusting for targetHeight";
    #endif
        scaleMatrix.scale (1, double (targetHeight) / double (newRect.height ()));
    }

    if (!scaleMatrix.isIdentity ())
    {
    #if DEBUG_KP_PIXMAP_FX && 1
        // TODO: What is going on here???  Why isn't matrix * working properly?
        QMatrix wrongMatrix = transformMatrix * scaleMatrix;
        QMatrix oldHat = transformMatrix;
        if (targetWidth > 0 && targetWidth != newRect.width ())
            oldHat.scale (double (targetWidth) / double (newRect.width ()), 1);
        if (targetHeight > 0 && targetHeight != newRect.height ())
            oldHat.scale (1, double (targetHeight) / double (newRect.height ()));
        QMatrix altHat = transformMatrix;
        altHat.scale ((targetWidth > 0 && targetWidth != newRect.width ()) ? double (targetWidth) / double (newRect.width ()) : 1,
                      (targetHeight > 0 && targetHeight != newRect.height ()) ? double (targetHeight) / double (newRect.height ()) : 1);
        QMatrix correctMatrix = scaleMatrix * transformMatrix;

        kDebug () << "\tsupposedlyWrongMatrix: m11=" << wrongMatrix.m11 ()  // <<<---- this is the correct matrix???
                   << " m12=" << wrongMatrix.m12 ()
                   << " m21=" << wrongMatrix.m21 ()
                   << " m22=" << wrongMatrix.m22 ()
                   << " dx=" << wrongMatrix.dx ()
                   << " dy=" << wrongMatrix.dy ()
                   << " rect=" << wrongMatrix.mapRect (pm.rect ())
                   << endl
                   << "\ti_used_to_use_thisMatrix: m11=" << oldHat.m11 ()
                   << " m12=" << oldHat.m12 ()
                   << " m21=" << oldHat.m21 ()
                   << " m22=" << oldHat.m22 ()
                   << " dx=" << oldHat.dx ()
                   << " dy=" << oldHat.dy ()
                   << " rect=" << oldHat.mapRect (pm.rect ())
                   << endl
                   << "\tabove but scaled at the same time: m11=" << altHat.m11 ()
                   << " m12=" << altHat.m12 ()
                   << " m21=" << altHat.m21 ()
                   << " m22=" << altHat.m22 ()
                   << " dx=" << altHat.dx ()
                   << " dy=" << altHat.dy ()
                   << " rect=" << altHat.mapRect (pm.rect ())
                   << endl
                   << "\tsupposedlyCorrectMatrix: m11=" << correctMatrix.m11 ()
                   << " m12=" << correctMatrix.m12 ()
                   << " m21=" << correctMatrix.m21 ()
                   << " m22=" << correctMatrix.m22 ()
                   << " dx=" << correctMatrix.dx ()
                   << " dy=" << correctMatrix.dy ()
                   << " rect=" << correctMatrix.mapRect (pm.rect ())
                   << endl;
    #endif

        transformMatrix = transformMatrix * scaleMatrix;

        newRect = transformMatrix.mapRect (pm.rect ());
    #if DEBUG_KP_PIXMAP_FX && 1
        kDebug () << "\tnewRect after targetWidth,targetHeight adjust=" << newRect;
    #endif
    }


    ::MatrixDebug ("TransformPixmap(): before trueMatrix", transformMatrix,
                   pm.width (), pm.height ());
#if DEBUG_KP_PIXMAP_FX && 1
    QMatrix oldMatrix = transformMatrix;
#endif

    // Translate the matrix to account for Qt rounding errors,
    // so that flipping (if it used this method) and rotating by a multiple
    // of 90 degrees actually work as expected (try tests/transforms.png).
    //
    // SYNC: This was not required with Qt3 so we are actually working
    //       around a Qt4 bug/feature.
    //
    // COMPAT: Qt4's rendering with a matrix enabled is low quality anyway
    //         but does this reduce quality even further?
    //
    //         With or without it, skews by 45 degrees with the QImage
    //         painter below look bad (with it, you get an extra transparent
    //         line on the right; without, you get only about 1/4 of a source
    //         line on the left).  In Qt3, with TrueMatrix(), the source
    //         image is translated 1 pixel off the destination image.
    //
    //         Also, if you skew a rectangular selection, the skewed selection
    //         border does not line up with the skewed image data. 
    // TODO: do we need to pass <newRect> through this new matrix?
    transformMatrix = ::TrueMatrix (transformMatrix,
        pm.width (), pm.height ());

#if DEBUG_KP_PIXMAP_FX && 1
    kDebug () << "trueMatrix changed matrix?" << (oldMatrix == transformMatrix);
#endif
    ::MatrixDebug ("TransformPixmap(): after trueMatrix", transformMatrix,
                   pm.width (), pm.height ());


    QImage newQImage (targetWidth > 0 ? targetWidth : newRect.width (),
                      targetHeight > 0 ? targetHeight : newRect.height (),
                      QImage::Format_ARGB32_Premultiplied);

    if ((targetWidth > 0 && targetWidth != newRect.width ()) ||
        (targetHeight > 0 && targetHeight != newRect.height ()))
    {
    #if DEBUG_KP_PIXMAP_FX && 1
        kDebug () << "kppixmapfx.cpp: TransformPixmap(pm.size=" << pm.size ()
                   << ",targetWidth=" << targetWidth
                   << ",targetHeight=" << targetHeight
                   << ") newRect=" << newRect
                   << " (you are a victim of rounding error)"
                   << endl;
    #endif
    }


#if DEBUG_KP_PIXMAP_FX && 0
    kDebug () << "\ttranslate top=" << painter.xForm (QPoint (0, 0));
    kDebug () << "\tmatrix: m11=" << painter.worldMatrix ().m11 ()
               << " m12=" << painter.worldMatrix ().m12 ()
               << " m21=" << painter.worldMatrix ().m21 ()
               << " m22=" << painter.worldMatrix ().m22 ()
               << " dx=" << painter.worldMatrix ().dx ()
               << " dy=" << painter.worldMatrix ().dy ()
               << endl;
#endif


    // Note: Do _not_ use "p.setRenderHints (QPainter::SmoothPixmapTransform);"
    //       as the user does not want their image to get blurier every
    //       time they e.g. rotate it (especially important for multiples
    //       of 90 degrees but also true for every other angle).  Being a
    //       pixel-based program, we generally like to preserve RGB values
    //       and avoid unnecessary blurs -- in the worst case, we'd rather
    //       drop pixels, than blur.
    QPainter p (&newQImage);
    {
        // Make sure transparent pixels are drawn into the destination image.
        p.setCompositionMode (QPainter::CompositionMode_Source);
    
        // Fill the entire new image with the background color.
        if (backgroundColor.isValid ())
        {
            p.fillRect (newQImage.rect (), backgroundColor.toQColor ());
        }

        p.setMatrix (transformMatrix);
        p.drawImage (QPoint (0, 0), pm);
    }
    p.end ();

#if DEBUG_KP_PIXMAP_FX && 1
    kDebug () << "Done" << endl << endl;
#endif

    return newQImage;
}

//---------------------------------------------------------------------

// public static
QMatrix kpPixmapFX::skewMatrix (int width, int height, double hangle, double vangle)
{
    if (fabs (hangle - 0) < kpPixmapFX::AngleInDegreesEpsilon &&
        fabs (vangle - 0) < kpPixmapFX::AngleInDegreesEpsilon)
    {
        return QMatrix ();
    }


    /* Diagram for completeness :)
     *
     *       |---------- w ----------|
     *     (0,0)
     *  _     _______________________ (w,0)
     *  |    |\~_ va                 |
     *  |    | \ ~_                  |
     *  |    |ha\  ~__               |
     *       |   \    ~__            | dy
     *  h    |    \      ~___        |
     *       |     \         ~___    |
     *  |    |      \            ~___| (w,w*tan(va)=dy)
     *  |    |       \         *     \
     *  _    |________\________|_____|\                                     vertical shear factor
     *     (0,h) dx   ^~_      |       \                                             |
     *                |  ~_    \________\________ General Point (x,y)                V
     *                |    ~__           \        Skewed Point (x + y*tan(ha),y + x*tan(va))
     *      (h*tan(ha)=dx,h)  ~__         \                             ^
     *                           ~___      \                            |
     *                               ~___   \                   horizontal shear factor
     *   Key:                            ~___\
     *    ha = hangle                         (w + h*tan(ha)=w+dx,h + w*tan(va)=w+dy)
     *    va = vangle
     *
     * Skewing really just twists a rectangle into a parallelogram.
     *
     */

    //QMatrix matrix (1, tan (KP_DEGREES_TO_RADIANS (vangle)), tan (KP_DEGREES_TO_RADIANS (hangle)), 1, 0, 0);
    // I think this is clearer than above :)
    QMatrix matrix;
    matrix.shear (tan (KP_DEGREES_TO_RADIANS (hangle)),
                  tan (KP_DEGREES_TO_RADIANS (vangle)));

    return ::MatrixWithZeroOrigin (matrix, width, height);
}

//---------------------------------------------------------------------

// public static
QMatrix kpPixmapFX::skewMatrix (const QImage &pixmap, double hangle, double vangle)
{
    return kpPixmapFX::skewMatrix (pixmap.width (), pixmap.height (), hangle, vangle);
}

//---------------------------------------------------------------------


// public static
void kpPixmapFX::skew (QImage *destPtr, double hangle, double vangle,
                       const kpColor &backgroundColor,
                       int targetWidth, int targetHeight)
{
    if (!destPtr)
        return;

    *destPtr = kpPixmapFX::skew (*destPtr, hangle, vangle,
                                       backgroundColor,
                                       targetWidth, targetHeight);
}

//---------------------------------------------------------------------

// public static
QImage kpPixmapFX::skew (const QImage &pm, double hangle, double vangle,
                          const kpColor &backgroundColor,
                          int targetWidth, int targetHeight)
{
#if DEBUG_KP_PIXMAP_FX
    kDebug () << "kpPixmapFX::skew() pm.width=" << pm.width ()
               << " pm.height=" << pm.height ()
               << " hangle=" << hangle
               << " vangle=" << vangle
               << " targetWidth=" << targetWidth
               << " targetHeight=" << targetHeight
               << endl;
#endif

    if (fabs (hangle - 0) < kpPixmapFX::AngleInDegreesEpsilon &&
        fabs (vangle - 0) < kpPixmapFX::AngleInDegreesEpsilon &&
        (targetWidth <= 0 && targetHeight <= 0)/*don't want to scale?*/)
    {
        return pm;
    }

    if (fabs (hangle) > 90 - kpPixmapFX::AngleInDegreesEpsilon ||
        fabs (vangle) > 90 - kpPixmapFX::AngleInDegreesEpsilon)
    {
        kError () << "kpPixmapFX::skew() passed hangle and/or vangle out of range (-90 < x < 90)" << endl;
        return pm;
    }


    QMatrix matrix = skewMatrix (pm, hangle, vangle);

    return ::TransformPixmap (pm, matrix, backgroundColor, targetWidth, targetHeight);
}

//---------------------------------------------------------------------


// public static
QMatrix kpPixmapFX::rotateMatrix (int width, int height, double angle)
{
    if (fabs (angle - 0) < kpPixmapFX::AngleInDegreesEpsilon)
    {
        return QMatrix ();
    }

    QMatrix matrix;
    matrix.translate (width / 2, height / 2);
    matrix.rotate (angle);

    return ::MatrixWithZeroOrigin (matrix, width, height);
}

//---------------------------------------------------------------------

// public static
QMatrix kpPixmapFX::rotateMatrix (const QImage &pixmap, double angle)
{
    return kpPixmapFX::rotateMatrix (pixmap.width (), pixmap.height (), angle);
}

//---------------------------------------------------------------------


// public static
bool kpPixmapFX::isLosslessRotation (double angle)
{
    const double angleIn = angle;

    // Reflect angle into positive if negative
    if (angle < 0)
        angle = -angle;

    // Remove multiples of 90 to make sure 0 <= angle <= 90
    angle -= ((int) angle) / 90 * 90;

    // "Impossible" situation?
    if (angle < 0 || angle > 90)
    {
        kError () << "kpPixmapFX::isLosslessRotation(" << angleIn
                   << ") result=" << angle
                   << endl;
        return false;  // better safe than sorry
    }

    const bool ret = (angle < kpPixmapFX::AngleInDegreesEpsilon ||
                      90 - angle < kpPixmapFX::AngleInDegreesEpsilon);
#if DEBUG_KP_PIXMAP_FX
    kDebug () << "kpPixmapFX::isLosslessRotation(" << angleIn << ")"
               << "  residual angle=" << angle
               << "  returning " << ret
               << endl;
#endif
    return ret;
}

//---------------------------------------------------------------------


// public static
void kpPixmapFX::rotate (QImage *destPtr, double angle,
                         const kpColor &backgroundColor,
                         int targetWidth, int targetHeight)
{
    if (!destPtr)
        return;

    *destPtr = kpPixmapFX::rotate (*destPtr, angle,
                                         backgroundColor,
                                         targetWidth, targetHeight);
}

//---------------------------------------------------------------------

// public static
QImage kpPixmapFX::rotate (const QImage &pm, double angle,
                            const kpColor &backgroundColor,
                            int targetWidth, int targetHeight)
{
    if (fabs (angle - 0) < kpPixmapFX::AngleInDegreesEpsilon &&
        (targetWidth <= 0 && targetHeight <= 0)/*don't want to scale?*/)
    {
        return pm;
    }


    QMatrix matrix = rotateMatrix (pm, angle);

    return ::TransformPixmap (pm, matrix, backgroundColor, targetWidth, targetHeight);
}

//---------------------------------------------------------------------
