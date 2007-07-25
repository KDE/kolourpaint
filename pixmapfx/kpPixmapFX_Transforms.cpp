
// COMPAT: Port to Qt4
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

#include <qapplication.h>
#include <qbitmap.h>
#include <qdatetime.h>
#include <qimage.h>
#include <qpainter.h>
#include <qpainterpath.h>
#include <qpixmap.h>
#include <qpoint.h>
#include <qpolygon.h>
#include <qrect.h>

#include <kconfig.h>
#include <kconfiggroup.h>
#include <kdebug.h>
#include <kglobal.h>
#include <klocale.h>
#include <kmessagebox.h>

#include <kpAbstractSelection.h>
#include <kpColor.h>
#include <kpDefs.h>
#include <kpTool.h>


// public static
void kpPixmapFX::resize (QPixmap *destPixmapPtr, int w, int h,
                         const kpColor &backgroundColor)
{
#if DEBUG_KP_PIXMAP_FX && 1
    kDebug () << "kpPixmapFX::resize()" << endl;
#endif

    if (!destPixmapPtr)
        return;

    KP_PFX_CHECK_NO_ALPHA_CHANNEL (*destPixmapPtr);

    const int oldWidth = destPixmapPtr->width ();
    const int oldHeight = destPixmapPtr->height ();

    if (w == oldWidth && h == oldHeight)
        return;


    QPixmap newPixmap (w, h);

    // Would have new undefined areas?
    if (w > oldWidth || h > oldHeight)
    {
    #if DEBUG_KP_PIXMAP_FX && 1
        kDebug () << "\tbacking with fill opqaque="
                  << backgroundColor.isOpaque () << endl;
    #endif
        if (backgroundColor.isOpaque ())
            newPixmap.fill (backgroundColor.toQColor ());
        else
        {
            QBitmap newPixmapMask (w, h);
            newPixmapMask.fill (Qt::color0/*transparent*/);
            newPixmap.setMask (newPixmapMask);
        }
    }

    // Copy over old pixmap.
    setPixmapAt (&newPixmap, 0, 0, *destPixmapPtr);

    // Replace pixmap with new one.
    *destPixmapPtr = newPixmap;

    KP_PFX_CHECK_NO_ALPHA_CHANNEL (*destPixmapPtr);
}

// public static
QPixmap kpPixmapFX::resize (const QPixmap &pm, int w, int h,
                            const kpColor &backgroundColor)
{
    QPixmap ret = pm;
    kpPixmapFX::resize (&ret, w, h, backgroundColor);
    return ret;
}


// public static
void kpPixmapFX::scale (QPixmap *destPixmapPtr, int w, int h, bool pretty)
{
    if (!destPixmapPtr)
        return;

    *destPixmapPtr = kpPixmapFX::scale (*destPixmapPtr, w, h, pretty);
}

// public static
QPixmap kpPixmapFX::scale (const QPixmap &pm, int w, int h, bool pretty)
{
    QPixmap retPixmap;

#if DEBUG_KP_PIXMAP_FX && 0
    kDebug () << "kpPixmapFX::scale(oldRect=" << pm.rect ()
               << ",w=" << w
               << ",h=" << h
               << ",pretty=" << pretty
               << ")"
               << endl;
#endif

    // HACK: We're used by kpTransformResizeScaleDialog.cpp:toolButtonIconSet()
    //       to scale an icon.  Rethink whether we really need to enforce no
    //       alpha channel in kpPixmapFX.
    // KP_PFX_CHECK_NO_ALPHA_CHANNEL (pm);

    if (w == pm.width () && h == pm.height ())
        return pm;


    if (pretty)
    {
        QImage image = kpPixmapFX::convertToImage (pm);

    #if DEBUG_KP_PIXMAP_FX && 0
        kDebug () << "\tBefore smooth scale:" << endl;
        for (int y = 0; y < image.height (); y++)
        {
            for (int x = 0; x < image.width (); x++)
            {
                fprintf (stderr, " %08X", image.pixel (x, y));
            }
            fprintf (stderr, "\n");
        }
    #endif

        image = image.scaled (w, h, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);

    #if DEBUG_KP_PIXMAP_FX && 0
        kDebug () << "\tAfter smooth scale:" << endl;
        for (int y = 0; y < image.height (); y++)
        {
            for (int x = 0; x < image.width (); x++)
            {
                fprintf (stderr, " %08X", image.pixel (x, y));
            }
            fprintf (stderr, "\n");
        }
    #endif

        retPixmap = kpPixmapFX::convertToPixmap (image, false/*let's not smooth it again*/);
    }
    else
    {
        QMatrix matrix;

        // COMPAT: mask as well?
        matrix.scale (double (w) / double (pm.width ()),
                      double (h) / double (pm.height ()));

        retPixmap = pm.transformed (matrix);
    }


    // HACK: We're used by kpTransformResizeScaleDialog.cpp:toolButtonIconSet()
    //       to scale an icon.  Rethink whether we really need to enforce no
    //       alpha channel in kpPixmapFX.
    // KP_PFX_CHECK_NO_ALPHA_CHANNEL (retPixmap);
    return retPixmap;
}


// public static
const double kpPixmapFX::AngleInDegreesEpsilon =
    KP_RADIANS_TO_DEGREES (atan (1.0 / 10000.0))
        / (2.0/*max error allowed*/ * 2.0/*for good measure*/);


static QMatrix matrixWithZeroOrigin (const QMatrix &matrix, int width, int height)
{
    QRect newRect = matrix.mapRect (QRect (0, 0, width, height));

    QMatrix translatedMatrix (matrix.m11 (), matrix.m12 (), matrix.m21 (), matrix.m22 (),
                               matrix.dx () - newRect.left (), matrix.dy () - newRect.top ());

    return translatedMatrix;
}

static QPixmap xForm (const QPixmap &pm, const QMatrix &transformMatrix_,
                      const kpColor &backgroundColor,
                      int targetWidth, int targetHeight)
{
    QMatrix transformMatrix = transformMatrix_;

#if DEBUG_KP_PIXMAP_FX && 1
    kDebug () << "kppixmapfx.cpp: xForm(pm.size=" << pm.size ()
               << ",targetWidth=" << targetWidth
               << ",targetHeight=" << targetHeight
               << ")"
               << endl;
#endif
    QRect newRect = transformMatrix.mapRect (pm.rect ());
#if DEBUG_KP_PIXMAP_FX && 1
    kDebug () << "\tmappedRect=" << newRect << endl;

#endif

    QMatrix scaleMatrix;
    if (targetWidth > 0 && targetWidth != newRect.width ())
    {
    #if DEBUG_KP_PIXMAP_FX && 1
        kDebug () << "\tadjusting for targetWidth" << endl;
    #endif
        scaleMatrix.scale (double (targetWidth) / double (newRect.width ()), 1);
    }

    if (targetHeight > 0 && targetHeight != newRect.height ())
    {
    #if DEBUG_KP_PIXMAP_FX && 1
        kDebug () << "\tadjusting for targetHeight" << endl;
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
        kDebug () << "\tnewRect after targetWidth,targetHeight adjust=" << newRect << endl;
    #endif
    }


    QPixmap newPixmap (targetWidth > 0 ? targetWidth : newRect.width (),
                       targetHeight > 0 ? targetHeight : newRect.height ());
    if ((targetWidth > 0 && targetWidth != newRect.width ()) ||
        (targetHeight > 0 && targetHeight != newRect.height ()))
    {
    #if DEBUG_KP_PIXMAP_FX && 1
        kDebug () << "kppixmapfx.cpp: xForm(pm.size=" << pm.size ()
                   << ",targetWidth=" << targetWidth
                   << ",targetHeight=" << targetHeight
                   << ") newRect=" << newRect
                   << " (you are a victim of rounding error)"
                   << endl;
    #endif
    }

    QBitmap newBitmapMask;

    if (backgroundColor.isOpaque ())
        newPixmap.fill (backgroundColor.toQColor ());

    if (backgroundColor.isTransparent () || !pm.mask ().isNull ())
    {
        newBitmapMask = QPixmap (newPixmap.width (), newPixmap.height ());
        newBitmapMask.fill (backgroundColor.maskColor ());
    }

    QPainter painter (&newPixmap);
#if DEBUG_KP_PIXMAP_FX && 0
    kDebug () << "\tmatrix: m11=" << transformMatrix.m11 ()
            << " m12=" << transformMatrix.m12 ()
            << " m21=" << transformMatrix.m21 ()
            << " m22=" << transformMatrix.m22 ()
            << " dx=" << transformMatrix.dx ()
            << " dy=" << transformMatrix.dy ()
            << endl;
#endif
    painter.setMatrix (transformMatrix);
#if DEBUG_KP_PIXMAP_FX && 0
    kDebug () << "\ttranslate top=" << painter.xForm (QPoint (0, 0)) << endl;
    kDebug () << "\tmatrix: m11=" << painter.worldMatrix ().m11 ()
               << " m12=" << painter.worldMatrix ().m12 ()
               << " m21=" << painter.worldMatrix ().m21 ()
               << " m22=" << painter.worldMatrix ().m22 ()
               << " dx=" << painter.worldMatrix ().dx ()
               << " dy=" << painter.worldMatrix ().dy ()
               << endl;
#endif
    painter.drawPixmap (QPoint (0, 0), pm);
    painter.end ();

    if (!newBitmapMask.isNull ())
    {
        QPainter maskPainter (&newBitmapMask);
        maskPainter.setMatrix (transformMatrix);
        maskPainter.drawPixmap (QPoint (0, 0), kpPixmapFX::getNonNullMask (pm));
        maskPainter.end ();
        newPixmap.setMask (newBitmapMask);
    }

    return newPixmap;
}

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

    //QWMatrix matrix (1, tan (KP_DEGREES_TO_RADIANS (vangle)), tan (KP_DEGREES_TO_RADIANS (hangle)), 1, 0, 0);
    // I think this is clearer than above :)
    QMatrix matrix;
    matrix.shear (tan (KP_DEGREES_TO_RADIANS (hangle)),
                  tan (KP_DEGREES_TO_RADIANS (vangle)));

    return matrixWithZeroOrigin (matrix, width, height);
}

// public static
QMatrix kpPixmapFX::skewMatrix (const QPixmap &pixmap, double hangle, double vangle)
{
    return kpPixmapFX::skewMatrix (pixmap.width (), pixmap.height (), hangle, vangle);
}


// public static
void kpPixmapFX::skew (QPixmap *destPixmapPtr, double hangle, double vangle,
                       const kpColor &backgroundColor,
                       int targetWidth, int targetHeight)
{
    if (!destPixmapPtr)
        return;

    *destPixmapPtr = kpPixmapFX::skew (*destPixmapPtr, hangle, vangle,
                                       backgroundColor,
                                       targetWidth, targetHeight);
}

// public static
QPixmap kpPixmapFX::skew (const QPixmap &pm, double hangle, double vangle,
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

    return ::xForm (pm, matrix, backgroundColor, targetWidth, targetHeight);
}


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

    return matrixWithZeroOrigin (matrix, width, height);
}

// public static
QMatrix kpPixmapFX::rotateMatrix (const QPixmap &pixmap, double angle)
{
    return kpPixmapFX::rotateMatrix (pixmap.width (), pixmap.height (), angle);
}


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


// public static
void kpPixmapFX::rotate (QPixmap *destPixmapPtr, double angle,
                         const kpColor &backgroundColor,
                         int targetWidth, int targetHeight)
{
    if (!destPixmapPtr)
        return;

    *destPixmapPtr = kpPixmapFX::rotate (*destPixmapPtr, angle,
                                         backgroundColor,
                                         targetWidth, targetHeight);
}

// public static
QPixmap kpPixmapFX::rotate (const QPixmap &pm, double angle,
                            const kpColor &backgroundColor,
                            int targetWidth, int targetHeight)
{
    if (fabs (angle - 0) < kpPixmapFX::AngleInDegreesEpsilon &&
        (targetWidth <= 0 && targetHeight <= 0)/*don't want to scale?*/)
    {
        return pm;
    }


    QMatrix matrix = rotateMatrix (pm, angle);

    return ::xForm (pm, matrix, backgroundColor, targetWidth, targetHeight);
}


// public static
QMatrix kpPixmapFX::flipMatrix (int width, int height, bool horz, bool vert)
{
    if (width <= 0 || height <= 0)
    {
        kError () << "kpPixmapFX::flipMatrix() passed invalid dimensions" << endl;
        return QMatrix ();
    }

    return QMatrix (horz ? -1 : +1,  // m11
                     0,  // m12
                     0,  // m21
                     vert ? -1 : +1,  // m22
                     horz ? (width - 1) : 0,  // dx
                     vert ? (height - 1) : 0);  // dy
}

// public static
QMatrix kpPixmapFX::flipMatrix (const QPixmap &pixmap, bool horz, bool vert)
{
    return kpPixmapFX::flipMatrix (pixmap.width (), pixmap.height (),
                                   horz, vert);
}


// public static
void kpPixmapFX::flip (QPixmap *destPixmapPtr, bool horz, bool vert)
{
    if (!horz && !vert)
        return;

    *destPixmapPtr = kpPixmapFX::flip (*destPixmapPtr, horz, vert);
}

// public static
QPixmap kpPixmapFX::flip (const QPixmap &pm, bool horz, bool vert)
{
    if (!horz && !vert)
        return pm;

    return pm.transformed (flipMatrix (pm, horz, vert));
}

// public static
void kpPixmapFX::flip (QImage *destImagePtr, bool horz, bool vert)
{
    if (!horz && !vert)
        return;

    *destImagePtr = kpPixmapFX::flip (*destImagePtr, horz, vert);
}

// public static
QImage kpPixmapFX::flip (const QImage &img, bool horz, bool vert)
{
    if (!horz && !vert)
        return img;

    return img.mirrored (horz, vert);
}
