
/* This file is part of the KolourPaint project
   Copyright (c) 2003 Clarence Dang <dang@kde.org>
   All rights reserved.

   Redistribution and use in source and binary forms, with or without
   modification, are permitted provided that the following conditions
   are met:

   1. Redistributions of source code must retain the above copyright
      notice, this list of conditions and the following disclaimer.
   2. Redistributions in binary form must reproduce the above copyright
      notice, this list of conditions and the following disclaimer in the
      documentation and/or other materials provided with the distribution.
   3. Neither the names of the copyright holders nor the names of
      contributors may be used to endorse or promote products derived from
      this software without specific prior written permission.

   THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
   "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
   LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A
   PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
   HOLDERS OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
   SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
   LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
   DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
   THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
   (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
   OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#define DEBUG_KP_PIXMAP_FX 1

#include <qbitmap.h>
#include <qimage.h>
#include <qpainter.h>
#include <qpixmap.h>
#include <qpoint.h>
#include <qrect.h>

#include <kdebug.h>

#include <kpdefs.h>
#include <kppixmapfx.h>
#include <kptool.h>


/*
 * QPixmap/QImage Conversion Functions
 */

 // public static
 QImage kpPixmapFX::convertToImage (const QPixmap &pixmap)
 {
     return pixmap.convertToImage ();
 }

 // public static
 QPixmap kpPixmapFX::convertToPixmap (const QImage &image, bool pretty)
 {
     QPixmap destPixmap;

     if (!pretty)
     {
         destPixmap.convertFromImage (image,
                                      Qt::ColorOnly/*always display depth*/ |
                                      Qt::ThresholdDither/*no dither*/ |
                                      Qt::ThresholdAlphaDither/*no dither alpha*/|
                                      Qt::AvoidDither);
     }
     else
     {
         destPixmap.convertFromImage (image,
                                      Qt::ColorOnly/*always display depth*/ |
                                      Qt::DiffuseDither/*hi quality dither*/ |
                                      Qt::ThresholdAlphaDither/*no dither alpha*/ |
                                      Qt::PreferDither/*(dither even if <256 colours???)*/);
     }

     kpPixmapFX::ensureNoAlphaChannel (&destPixmap);
     return destPixmap;
 }


/*
 * Get/Set Parts of Pixmap
 */


// public static
QPixmap kpPixmapFX::getPixmapAt (const QPixmap &pm, const QRect &rect)
{
    QPixmap retPixmap (rect.width (), rect.height ());

#if DEBUG_KP_PIXMAP_FX && 0
    kdDebug () << "kpPixmapFX::getPixmapAt(pm.hasMask="
               << (pm.mask () ? 1 : 0)
               << ",rect="
               << rect
               << ")"
               << endl;
#endif

    // copy data _and_ mask (if avail)
    copyBlt (&retPixmap, /* dest */
             0, 0, /* dest pt */
             &pm, /* src */
             rect.x (), rect.y (), /* src pt */
             rect.width (), rect.height ());

#if DEBUG_KP_PIXMAP_FX && 0
    kdDebug () << "\tretPixmap.hasMask="
               << (retPixmap.mask () ? 1 : 0)
               << endl;
#endif

    return retPixmap;
}

// public static
void kpPixmapFX::setPixmapAt (QPixmap *destPixmapPtr, const QRect &destRect,
                              const QPixmap &srcPixmap)
{
    if (!destPixmapPtr)
        return;

#if DEBUG_KP_PIXMAP_FX && 0
    kdDebug () << "kpPixmapFX::setPixmapAt(destPixmap->rect="
               << destPixmapPtr->rect ()
               << ",destPixmap->hasMask="
               << (destPixmapPtr->mask () ? 1 : 0)
               << ",destRect="
               << destRect
               << ",srcPixmap.rect="
               << srcPixmap.rect ()
               << ",srcPixmap.hasMask="
               << (srcPixmap.mask () ? 1 : 0)
               << ")"
               << endl;
#endif

#if DEBUG_KP_PIXMAP_FX && 0
    if (destPixmapPtr->mask ())
    {
        QImage image = kpPixmapFX::convertToImage (*destPixmapPtr);
        int numTrans = 0;

        for (int y = 0; y < image.height (); y++)
        {
            for (int x = 0; x < image.width (); x++)
            {
                if (qAlpha (image.pixel (x, y)) == 0)
                    numTrans++;
            }
        }

        kdDebug () << "\tdestPixmapPtr numTrans=" << numTrans << endl;
    }
#endif

#if 0
    // TODO: why does undo'ing a single pen dot on a transparent pixel,
    //       result in a opaque image, except for that single transparent pixel???
    //       Qt bug on boundary case?

    // copy data _and_ mask
    copyBlt (destPixmapPtr,
             destAt.x (), destAt.y (),
             &srcPixmap,
             0, 0,
             destRect.width (), destRect.height ());
#else
    bitBlt (destPixmapPtr,
            destRect.x (), destRect.y (),
            &srcPixmap,
            0, 0,
            destRect.width (), destRect.height (),
            Qt::CopyROP,
            true/*ignore mask*/);

    if (srcPixmap.mask ())
    {
        QBitmap mask = getNonNullMask (*destPixmapPtr);
        bitBlt (&mask,
                destRect.x (), destRect.y (),
                srcPixmap.mask (),
                0, 0,
                destRect.width (), destRect.height (),
                Qt::CopyROP,
                true/*ignore mask*/);
        destPixmapPtr->setMask (mask);
    }
#endif

    if (destPixmapPtr->mask () && !srcPixmap.mask ())
    {
    #if DEBUG_KP_PIXMAP_FX && 0
        kdDebug () << "\t\topaque'ing dest rect" << endl;
    #endif
        kpPixmapFX::ensureOpaqueAt (destPixmapPtr, destRect);
    }

#if DEBUG_KP_PIXMAP_FX && 0
    kdDebug () << "\tdestPixmap->hasMask="
               << (destPixmapPtr->mask () ? 1 : 0)
               << endl;
    if (destPixmapPtr->mask ())
    {
        QImage image = kpPixmapFX::convertToImage (*destPixmapPtr);
        int numTrans = 0;

        for (int y = 0; y < image.height (); y++)
        {
            for (int x = 0; x < image.width (); x++)
            {
                if (qAlpha (image.pixel (x, y)) == 0)
                    numTrans++;
            }
        }

        kdDebug () << "\tdestPixmapPtr numTrans=" << numTrans << endl;
    }
#endif
}

// public static
void kpPixmapFX::setPixmapAt (QPixmap *destPixmapPtr, const QPoint &destAt,
                              const QPixmap &srcPixmap)
{
    kpPixmapFX::setPixmapAt (destPixmapPtr,
                             QRect (destAt.x (), destAt.y (),
                                    srcPixmap.width (), srcPixmap.height ()),
                             srcPixmap);
}

// public static
void kpPixmapFX::paintPixmapAt (QPixmap *destPixmapPtr, const QPoint &destAt,
                                const QPixmap &srcPixmap)
{
    if (!destPixmapPtr)
        return;

    // copy data from dest onto the top of src (masked by src's mask)
    bitBlt (destPixmapPtr, /* dest */
            destAt.x (), destAt.y (), /* dest pt */
            &srcPixmap, /* src */
            0, 0 /* src pt */);

    kpPixmapFX::ensureOpaqueAt (destPixmapPtr, destAt, srcPixmap);
}

// public static
QColor kpPixmapFX::getColorAtPixel (const QPixmap &pm, const QPoint &at)
{
#if DEBUG_KP_PIXMAP_FX && 0
    kdDebug () << "kpToolColorPicker::colorAtPixel" << p << endl;
#endif

    QPixmap pixmap = getPixmapAt (pm, QRect (at, at));
    QImage image = kpPixmapFX::convertToImage (pixmap);
    if (image.isNull ())
    {
        kdError () << "kpPixmapFX::getColorAtPixel(QPixmap) could not convert to QImage" << endl;
        return QColor ();  // transparent
    }

    return getColorAtPixel (image, QPoint (0, 0));
}

// public static
QColor kpPixmapFX::getColorAtPixel (const QPixmap &pm, int x, int y)
{
    return kpPixmapFX::getColorAtPixel (pm, QPoint (x, y));
}

// public static
QColor kpPixmapFX::getColorAtPixel (const QImage &img, const QPoint &at)
{
    QRgb rgba = img.pixel (at.x (), at.y ());

    if (qAlpha (rgba) > 0)
    {
    #if DEBUG_KP_PIXMAP_FX && 0
        kdDebug () << "\topaque val=" << qAlpha (rgba)/*hopefully 255*/ << endl;
    #endif
        return QColor (rgba & RGB_MASK);
    }
    else
    {
    #if DEBUG_KP_PIXMAP_FX && 0
        kdDebug () << "\ttransparent" << endl;
    #endif
        return QColor ();  // transparent
    }
}

// public static
QColor kpPixmapFX::getColorAtPixel (const QImage &img, int x, int y)
{
    return kpPixmapFX::getColorAtPixel (img, QPoint (x, y));
}


/*
 * Mask Operations
 */


// public static
void kpPixmapFX::ensureNoAlphaChannel (QPixmap *destPixmapPtr)
{
    if (destPixmapPtr->hasAlphaChannel ())
        destPixmapPtr->setMask (kpPixmapFX::getNonNullMask/*just in case*/ (*destPixmapPtr));
}

// public static
QBitmap kpPixmapFX::getNonNullMask (const QPixmap &pm)
{
    if (pm.mask ())
        return *pm.mask ();
    else
    {
        QBitmap maskBitmap (pm.width (), pm.height ());
        maskBitmap.fill (Qt::color1/*opaque*/);

        return maskBitmap;
    }
}

// public static
QBitmap kpPixmapFX::getNonNullMaskAt (const QPixmap &pm, const QRect &rect)
{
    QBitmap destMaskBitmap (rect.width (), rect.height ());

    if (pm.mask ())
    {
        copyBlt (&destMaskBitmap, 0, 0,
                 &pm, rect.x (), rect.y (), rect.width (), rect.height ());
    }
    else
    {
        destMaskBitmap.fill (Qt::color1/*opaque*/);
    }

    return destMaskBitmap;
}

// public static
void kpPixmapFX::setMaskAt (QPixmap *destPixmapPtr, const QPoint &destAt,
                            const QBitmap &srcMaskBitmap)
{
    if (!destPixmapPtr)
        return;

    QBitmap destMaskBitmap (srcMaskBitmap.width (), srcMaskBitmap.height ());

    copyBlt (&destMaskBitmap, destAt.x (), destAt.y (),
             &srcMaskBitmap);

    destPixmapPtr->setMask (destMaskBitmap);
}

// public static
void kpPixmapFX::ensureTransparentAt (QPixmap *destPixmapPtr, const QRect &destRect)
{
    if (!destPixmapPtr)
        return;

    QBitmap maskBitmap = getNonNullMask (*destPixmapPtr);

    QPainter p (&maskBitmap);

    p.setPen (Qt::color0/*transparent*/);
    p.setBrush (Qt::color0/*transparent*/);

    p.drawRect (destRect);

    p.end ();

    destPixmapPtr->setMask (maskBitmap);
}

// public static
void kpPixmapFX::paintMaskTransparentWithBrush (QPixmap *destPixmapPtr, const QPoint &destAt,
                                                const QBitmap &brushBitmap)
{
    if (!destPixmapPtr)
        return;

    QBitmap destMaskBitmap = kpPixmapFX::getNonNullMask (*destPixmapPtr);

    //                  Src
    //  Dest Mask   Brush Bitmap   =   Result
    //  -------------------------------------
    //      0            0               0
    //      0            1               0
    //      1            0               1
    //      1            1               0
    //
    // Brush Bitmap value of 1 means "make transparent"
    //                       0 means "leave it as it is"

    bitBlt (&destMaskBitmap,
            destAt.x (), destAt.y (),
            &brushBitmap,
            0, 0,
            brushBitmap.width (), brushBitmap.height (),
            Qt::NotAndROP);

    destPixmapPtr->setMask (destMaskBitmap);
}

// public static
void kpPixmapFX::ensureOpaqueAt (QPixmap *destPixmapPtr, const QRect &destRect)
{
    if (!destPixmapPtr || !destPixmapPtr->mask ()/*already opaque*/)
        return;

    QBitmap maskBitmap = *destPixmapPtr->mask ();

    QPainter p (&maskBitmap);

    p.setPen (Qt::color1/*opaque*/);
    p.setBrush (Qt::color1/*opaque*/);

    p.drawRect (destRect);

    p.end ();

    destPixmapPtr->setMask (maskBitmap);
}

// public static
void kpPixmapFX::ensureOpaqueAt (QPixmap *destPixmapPtr, const QPoint &destAt,
                                 const QPixmap &srcPixmap)
{
    if (!destPixmapPtr || !destPixmapPtr->mask ()/*already opaque*/)
        return;

    QBitmap destMask = *destPixmapPtr->mask ();

    if (srcPixmap.mask ())
    {
        bitBlt (&destMask, /* dest */
                destAt, /* dest pt */
                srcPixmap.mask (), /* src */
                QRect (0, 0, srcPixmap.width (), srcPixmap.height ()), /* src rect */
                Qt::OrROP/*if either is opaque, it's opaque*/);
    }
    else
    {
        QPainter p (&destMask);

        p.setPen (Qt::color1/*opaque*/);
        p.setBrush (Qt::color1/*opaque*/);

        p.drawRect (destAt.x (), destAt.y (),
                    srcPixmap.width (), srcPixmap.height ());

        p.end ();
    }

    destPixmapPtr->setMask (destMask);
}


/*
 * Effects
 */

// public static
void kpPixmapFX::invertColors (QPixmap *destPixmapPtr)
{
    QImage image = kpPixmapFX::convertToImage (*destPixmapPtr);
    kpPixmapFX::invertColors (&image);
    *destPixmapPtr = kpPixmapFX::convertToPixmap (image);
}

// public static
QPixmap kpPixmapFX::invertColors (const QPixmap &pm)
{
    QImage image = kpPixmapFX::convertToImage (pm);
    kpPixmapFX::invertColors (&image);
    return kpPixmapFX::convertToPixmap (image);
}

// public static
void kpPixmapFX::invertColors (QImage *destImagePtr)
{
    if (destImagePtr->depth () > 8)
    {
    #if 0
        // SYNC: TODO: Qt BUG - invertAlpha argument is inverted!!!
        destImagePtr->invertPixels (true/*no invert alpha (Qt 3.2)*/);
    #else
        // Above version works for Qt 3.2 at least.
        // But this version will always work (slower, though):
        for (int y = 0; y < destImagePtr->height (); y++)
        {
            for (int x = 0; x < destImagePtr->width (); x++)
            {
                // flip RGB bits but not Alpha
                destImagePtr->setPixel (x, y, destImagePtr->pixel (x, y) ^ 0x00FFFFFF);
            }
        }
    #endif
    }
    else
    {
        for (int i = 0; i < destImagePtr->numColors (); i++)
        {
            // flip RGB bits but not Alpha
            destImagePtr->setColor (i, destImagePtr->color (i) ^ 0x00FFFFFF);
        }
    }
}

// public static
QImage kpPixmapFX::invertColors (const QImage &img)
{
    QImage retImage = img;
    kpPixmapFX::invertColors (&retImage);
    return retImage;
}


// public static
void kpPixmapFX::convertToGrayscale (QPixmap *destPixmapPtr)
{
    QImage image = kpPixmapFX::convertToImage (*destPixmapPtr);
    kpPixmapFX::convertToGrayscale (&image);
    *destPixmapPtr = kpPixmapFX::convertToPixmap (image);
}

// public static
QPixmap kpPixmapFX::convertToGrayscale (const QPixmap &pm)
{
    QImage image = kpPixmapFX::convertToImage (pm);
    kpPixmapFX::convertToGrayscale (&image);
    return kpPixmapFX::convertToPixmap (image);
}

static QRgb toGray (QRgb rgb)
{
    // naive way that doesn't preserve brightness
    // int gray = (qRed (rgb) + qGreen (rgb) + qBlue (rgb)) / 3;

    // over-exaggerates red & blue
    // int gray = qGray (rgb);

    int gray = (212671 * qRed (rgb) + 715160 * qGreen (rgb) + 72169 * qBlue (rgb)) / 1000000;
    return qRgba (gray, gray, gray, qAlpha (rgb));
}

// public static
void kpPixmapFX::convertToGrayscale (QImage *destImagePtr)
{
    if (destImagePtr->depth () > 8)
    {
        // hmm, why not just write to the pixmap directly???

        for (int y = 0; y < destImagePtr->height (); y++)
        {
            for (int x = 0; x < destImagePtr->width (); x++)
            {
                destImagePtr->setPixel (x, y, toGray (destImagePtr->pixel (x, y)));
            }
        }
    }
    else
    {
        // 1- & 8- bit images use a color table
        for (int i = 0; i < destImagePtr->numColors (); i++)
            destImagePtr->setColor (i, toGray (destImagePtr->color (i)));
    }
}

// public static
QImage kpPixmapFX::convertToGrayscale (const QImage &img)
{
    QImage retImage = img;
    kpPixmapFX::convertToGrayscale (&retImage);
    return retImage;
}


// public static
void kpPixmapFX::convertToBlackAndWhite (QPixmap *destPixmapPtr)
{
    QImage image = kpPixmapFX::convertToImage (*destPixmapPtr);
    if (!image.isNull ())
    {
    #if DEBUG_KP_PIXMAP_FX && 0
        for (int y = 0; y < image.width (); y++)
        {
            for (int x = 0; x < image.width (); x++)
            {
                fprintf (stderr, " %08X", image.pixel (x, y));
            }
            fprintf (stderr, "\n");
        }
    #endif

        image = image.convertDepth (1/*monochrome*/);

    #if DEBUG_KP_PIXMAP_FX && 0
        kdDebug () << "After conversion to B&W:" << endl;
        for (int y = 0; y < image.width (); y++)
        {
            for (int x = 0; x < image.width (); x++)
            {
                fprintf (stderr, " %08X", image.pixel (x, y));
            }
            fprintf (stderr, "\n");
        }
    #endif

        if (!image.isNull ())
        {
            QPixmap pixmap = kpPixmapFX::convertToPixmap (image, true/*pretty*/);

            // HACK: The above "image.convertDepth (1)" erases the Alpha Channel
            //       even if Qt::ColorOnly is specified in the conversion flags.
            //       qpixmap.html says "alpha masks on monochrome images are ignored."
            //
            //       Put the mask back.
            //
            if (destPixmapPtr->mask ())
                pixmap.setMask (*destPixmapPtr->mask ());

            *destPixmapPtr = pixmap;
        }
    }
}

// public static
QPixmap kpPixmapFX::convertToBlackAndWhite (const QPixmap &pm)
{
    QPixmap ret = pm;
    kpPixmapFX::convertToBlackAndWhite (&ret);
    return ret;
}


// public static
void kpPixmapFX::resize (QPixmap *destPixmapPtr, int w, int h,
                         const QColor &backgroundColor, bool fillNewAreas)
{
#if DEBUG_KP_PIXMAP_FX && 1
    kdDebug () << "kpPixmapFX::resize()" << endl;
#endif

    if (!destPixmapPtr)
        return;

    int oldWidth = destPixmapPtr->width ();
    int oldHeight = destPixmapPtr->height ();

    if (w == oldWidth && h == oldHeight)
        return;


    destPixmapPtr->resize (w, h);

    if (fillNewAreas && (w > oldWidth || h > oldHeight))
    {
    #if DEBUG_KP_PIXMAP_FX && 1
        kdDebug () << "\tfilling in new areas" << endl;
    #endif
        QBitmap maskBitmap;
        QPainter painter, maskPainter;

        if (kpTool::isColorOpaque (backgroundColor))
        {
            painter.begin (destPixmapPtr);
            painter.setPen (backgroundColor);
            painter.setBrush (backgroundColor);
        }

        if (kpTool::isColorTransparent (backgroundColor) || destPixmapPtr->mask ())
        {
            maskBitmap = kpPixmapFX::getNonNullMask (*destPixmapPtr);
            maskPainter.begin (&maskBitmap);
            if (kpTool::isColorTransparent (backgroundColor))
            {
                maskPainter.setPen (Qt::color0/*transparent*/);
                maskPainter.setBrush (Qt::color0/*transparent*/);
            }
            else
            {
                maskPainter.setPen (Qt::color1/*opaque*/);
                maskPainter.setBrush (Qt::color1/*opaque*/);
            }
        }

    #define PAINTER_CALL(cmd)         \
    {                                 \
        if (painter.isActive ())      \
            painter . cmd ;           \
                                      \
        if (maskPainter.isActive ())  \
            maskPainter . cmd ;       \
    }
        if (w > oldWidth)
            PAINTER_CALL (drawRect (oldWidth, 0, w - oldWidth, oldHeight));

        if (h > oldHeight)
            PAINTER_CALL (drawRect (0, oldHeight, w, h - oldHeight));
    #undef PAINTER_CALL

        if (maskPainter.isActive ())
            maskPainter.end ();

        if (painter.isActive ())
            painter.end ();

        if (!maskBitmap.isNull ())
            destPixmapPtr->setMask (maskBitmap);
    }
}

// public static
QPixmap kpPixmapFX::resize (const QPixmap &pm, int w, int h,
                            const QColor &backgroundColor, bool fillNewAreas)
{
    QPixmap ret = pm;
    kpPixmapFX::resize (&ret, w, h, backgroundColor, fillNewAreas);
    return ret;
}


// public static
void kpPixmapFX::scale (QPixmap *destPixmapPtr, int w, int h)
{
    if (!destPixmapPtr)
        return;

    *destPixmapPtr = kpPixmapFX::scale (*destPixmapPtr, w, h);
}

// public static
QPixmap kpPixmapFX::scale (const QPixmap &pm, int w, int h)
{
    if (w == pm.width () && h == pm.height ())
        return pm;

#if 0  // slow but smooth, requiring change to QImage (like QPixmap::xForm()?)
    QImage image = (kpPixmapFX::convertToImage (*m_pixmap)).smoothScale (w, h);

    if (!kpPixmapFX::convertToPixmap (image, true/*pretty*/))
    {
        kdError () << "kpDocument::scale() could not convertToPixmap()" << endl;
        return false;
    }
#else
    QWMatrix matrix;

    matrix.scale (double (w) / double (pm.width ()),
                  double (h) / double (pm.height ()));

    return pm.xForm (matrix);
#endif
}


// public static
void kpPixmapFX::skew (QPixmap *destPixmapPtr, double hangle, double vangle,
                       const QColor &backgroundColor)
{
    if (!destPixmapPtr)
        return;

    *destPixmapPtr = kpPixmapFX::skew (*destPixmapPtr, hangle, vangle,
                                       backgroundColor);
}

// public static
QPixmap kpPixmapFX::skew (const QPixmap &pm, double hangle, double vangle,
                          const QColor &backgroundColor)
{
#if DEBUG_KP_PIXMAP_FX
    kdDebug () << "kpPixmapFX::skew() CALLED" << endl;
#endif

    if (hangle == 0 && vangle == 0)
        return pm;

    // make sure -90 < hangle/vangle < 90 degrees:
    // if (abs (hangle) >= 90 || abs (vangle) >= 90) {
    // TODO: inconsistent
    if (90 - fabs (hangle) < KP_EPSILON || 90 - fabs (vangle) < KP_EPSILON)
    {
        kdError () << "kpPixmapFX::skew() passed hangle and/or vangle out of range (-90 < x < 90)" << endl;
        return pm;
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
    QWMatrix matrix;
    matrix.shear (tan (KP_DEGREES_TO_RADIANS (hangle)),
                  tan (KP_DEGREES_TO_RADIANS (vangle)));

    QRect newRect = matrix.mapRect (pm.rect ());
#if DEBUG_KP_PIXMAP_FX
    kdDebug () << "\tnewRect=" << newRect << endl;
#endif

    QWMatrix translatedMatrix (matrix.m11 (), matrix.m12 (), matrix.m21 (), matrix.m22 (),
                               -newRect.left (), -newRect.top ());

    QPixmap newPixmap (newRect.width (), newRect.height ());
    QBitmap newBitmapMask;

    if (kpTool::isColorOpaque (backgroundColor))
        newPixmap.fill (backgroundColor);

    if (kpTool::isColorTransparent (backgroundColor) || pm.mask ())
    {
        newBitmapMask.resize (newRect.width (), newRect.height ());
        newBitmapMask.fill (kpTool::isColorTransparent (backgroundColor)
                                ?
                            Qt::color0/*transparent*/
                                :
                            Qt::color1/*opaque*/);
    }

    QPainter painter (&newPixmap);
#if DEBUG_KP_PIXMAP_FX && 1
    kdDebug () << "\tmatrix: m11=" << matrix.m11 ()
            << " m12=" << matrix.m12 ()
            << " m21=" << matrix.m21 ()
            << " m22=" << matrix.m22 ()
            << " dx=" << matrix.dx ()
            << " dy=" << matrix.dy ()
            << endl;
#endif
    painter.setWorldMatrix (translatedMatrix);
#if DEBUG_KP_PIXMAP_FX && 1
    kdDebug () << "\ttranslate top=" << painter.xForm (QPoint (0, 0)) << endl;
    kdDebug () << "\tmatrix: m11=" << painter.worldMatrix ().m11 ()
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
        maskPainter.setWorldMatrix (translatedMatrix);
        maskPainter.drawPixmap (QPoint (0, 0), kpPixmapFX::getNonNullMask (pm));
        maskPainter.end ();
        newPixmap.setMask (newBitmapMask);
    }

    return newPixmap;
}


// public static
bool kpPixmapFX::isLosslessRotation (double angle)
{
    // TODO: we shouldn't round to int
    return (qRound (angle) % 90 == 0);
}


// public static
void kpPixmapFX::rotate (QPixmap *destPixmapPtr, double angle,
                         const QColor &backgroundColor)
{
    if (!destPixmapPtr)
        return;

    *destPixmapPtr = kpPixmapFX::rotate (*destPixmapPtr, angle,
                                         backgroundColor);
}

// public static
QPixmap kpPixmapFX::rotate (const QPixmap &pm, double angle,
                            const QColor &backgroundColor)
{
#if DEBUG_KP_PIXMAP_FX
    kdDebug () << "kpPixmapFX::rotate() CALLED" << endl;
#endif

    // TODO: epsilon
    if (angle == 0)
        return pm;

    if (angle < 0 || angle >= 360)
    {
        kdError () << "kpPixmapFX::rotate() passed angle ! >= 0 && < 360" << endl;
        return pm;
    }


    if (isLosslessRotation (angle))
    {
        QPixmap newPixmap;

        QWMatrix matrix;
        matrix.rotate (angle);

        newPixmap = pm.xForm (matrix);

        return newPixmap;
    }
    else
    {
        QWMatrix matrix;
        matrix.rotate (angle);  //360.0 - angle);  // TODO: not counterclockwise???

        // calculate size of new pixmap (allowing for rounding error)
        QRect newRect = matrix.mapRect (pm.rect ());
        newRect = QRect (0, 0, newRect.width () + 4, newRect.height () + 4);
    #if DEBUG_KP_PIXMAP_FX
        kdDebug () << "\tnewRect=" << newRect << endl;
    #endif

        // recalculate matrix - this time rotating old pixmap centred at the
        // middle of the new (probably bigger) pixmap
        matrix.reset ();
        matrix.translate (newRect.width () / 2, newRect.height () / 2);
        matrix.rotate (angle);
        matrix.translate (-newRect.width () / 2, -newRect.height () / 2);

        QRect srcRect ((newRect.width () - pm.width ()) / 2,
                       (newRect.height () - pm.height ()) / 2,
                       pm.width (),
                       pm.height ());

        QPixmap newPixmap (newRect.width (), newRect.height ());
        QBitmap newBitmapMask;

        if (kpTool::isColorOpaque (backgroundColor))
            newPixmap.fill (backgroundColor);

        if (kpTool::isColorTransparent (backgroundColor) || pm.mask ())
        {
            newBitmapMask.resize (newRect.width (), newRect.height ());
            newBitmapMask.fill (kpTool::isColorTransparent (backgroundColor)
                                    ?
                                Qt::color0/*transparent*/
                                    :
                                Qt::color1/*opaque*/);
        }

        QPoint drawPoint = srcRect.topLeft ();

        QPainter painter (&newPixmap);
        painter.setWorldMatrix (matrix);
    #if DEBUG_KP_PIXMAP_FX
        kdDebug () << "\tsrcRect=" << srcRect << endl;
    #endif
        QRect destRect = painter.xForm (srcRect);
    #if DEBUG_KP_PIXMAP_FX
        kdDebug () << "\tdestRect=" << destRect << endl;
    #endif

        painter.drawPixmap (drawPoint, pm);
        painter.end ();

        if (!newBitmapMask.isNull ())
        {
            QPainter maskPainter (&newBitmapMask);
            maskPainter.setWorldMatrix (matrix);
            maskPainter.drawPixmap (drawPoint, kpPixmapFX::getNonNullMask (pm));
            maskPainter.end ();
            newPixmap.setMask (newBitmapMask);
        }

        // get rid of extra border (that allowed for rounding errors)
        return kpPixmapFX::getPixmapAt (newPixmap, destRect);
    }
}


// public static
void kpPixmapFX::fill (QPixmap *destPixmapPtr, const QColor &color)
{
    if (!destPixmapPtr)
        return;

    if (kpTool::isColorOpaque (color))
    {
        destPixmapPtr->setMask (QBitmap ());  // no mask = opaque
        destPixmapPtr->fill (color);
    }
    else
    {
        kpPixmapFX::ensureTransparentAt (destPixmapPtr, destPixmapPtr->rect ());
    }
}

// public static
QPixmap kpPixmapFX::fill (const QPixmap &pm, const QColor &color)
{
    QPixmap ret = pm;
    kpPixmapFX::fill (&ret, color);
    return ret;
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

    QImage image = kpPixmapFX::convertToImage (pm);
    kpPixmapFX::flip (&image, horz, vert);
    return kpPixmapFX::convertToPixmap (image);
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

    return img.mirror (horz, vert);
}
