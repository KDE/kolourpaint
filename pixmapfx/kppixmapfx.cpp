
/*
   Copyright (c) 2003-2004 Clarence Dang <dang@kde.org>
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


#include <kppixmapfx.h>

#include <math.h>

#include <qapplication.h>
#include <qbitmap.h>
#include <qdatetime.h>
#include <qimage.h>
#include <qpainter.h>
#include <qpixmap.h>
#include <qpoint.h>
#include <qrect.h>

#include <kdebug.h>
#include <klocale.h>
#include <kmessagebox.h>

#include <kpcolor.h>
#include <kpdefs.h>
#include <kptool.h>


//
// QPixmap/QImage Conversion Functions
//

// public static
QImage kpPixmapFX::convertToImage (const QPixmap &pixmap)
{
    return pixmap.convertToImage ();
}


// Returns true if <image> contains translucency (rather than just transparency)
// QPixmap::hasAlphaChannel() appears to give incorrect results
static bool imageHasAlphaChannel (const QImage &image)
{
    if (image.depth () < 32)
        return false;

    for (int y = 0; y < image.height (); y++)
    {
        for (int x = 0; x < image.width (); x++)
        {
            const QRgb rgb = image.pixel (x, y);

            if (qAlpha (rgb) > 0 && qAlpha (rgb) < 255)
                return true;
        }
    }

    return false;
}

// Accurate but too slow
#if 0
static int imageNumColors (const QImage &image)
{
    QMap <QRgb, bool> rgbMap;

    if (image.depth () <= 8)
    {
        for (int i = 0; i < image.numColors () && rgbMap.size () < (1 << 16); i++)
            rgbMap.insert (image.color (i), true);
    }
    else
    {
        for (int y = 0; y < image.height () && rgbMap.size () < (1 << 16); y++)
        {
            for (int x = 0; x < image.width () && rgbMap.size () < (1 << 16); x++)
            {
                rgbMap.insert (image.pixel (x, y), true);
            }
        }
    }

    return rgbMap.size ();
}
#endif

static void convertToPixmapWarnAboutLoss (const QImage &image,
                                          const kpPixmapFX::WarnAboutLossInfo &wali)
{
    if (!wali.isValid ())
        return;


    const QString colorDepthTranslucencyDontAskAgain =
        wali.dontAskAgainPrefix () + "_ColorDepthTranslucency";
    const QString colorDepthDontAskAgain =
        wali.dontAskAgainPrefix () + "_ColorDepth";
    const QString translucencyDontAskAgain =
        wali.dontAskAgainPrefix () + "_Translucency";

#if DEBUG_KP_PIXMAP_FX && 1
    QTime timer;
    timer.start ();
#endif

    bool hasAlphaChannel =
        (KMessageBox::shouldBeShownContinue (translucencyDontAskAgain) &&
         imageHasAlphaChannel (image));

#if DEBUG_KP_PIXMAP_FX && 1
    kdDebug () << "\twarnAboutLoss - check hasAlphaChannel took "
               << timer.restart () << "msec" << endl;
#endif

    bool moreColorsThanDisplay =
        (KMessageBox::shouldBeShownContinue (colorDepthDontAskAgain) &&
         image.depth () > QColor::numBitPlanes () &&
         QColor::numBitPlanes () < 24);  // 32 indicates alpha channel

    int screenDepthNeeded = 0;

    if (moreColorsThanDisplay)
        screenDepthNeeded = QMIN (24, image.depth ());

#if DEBUG_KP_PIXMAP_FX && 1
    kdDebug () << "\ttranslucencyShouldBeShown="
                << KMessageBox::shouldBeShownContinue (translucencyDontAskAgain)
                << endl
                << "\thasAlphaChannel=" << hasAlphaChannel
                << endl
                << "\tcolorDepthShownBeShown="
                << KMessageBox::shouldBeShownContinue (colorDepthDontAskAgain)
                << endl
                << "\timage.depth()=" << image.depth ()
                << endl
                << "\tscreenDepth=" << QColor::numBitPlanes ()
                << endl
                << "\tmoreColorsThanDisplay=" << moreColorsThanDisplay
                << endl
                << "\tneedDepth=" << screenDepthNeeded
                << endl;
#endif


    QApplication::setOverrideCursor (Qt::arrowCursor);

    if (moreColorsThanDisplay && hasAlphaChannel)
    {
        KMessageBox::information (wali.parent (),
            i18n ("The %1 "
                  // sync: moreColorsThanDisplayMessage
                  "may have more colors than the current screen mode. "
                  "In order to display it, some colors may be changed. "
                  "Try increasing your screen "
                  "depth to at least %2bpp."

                  "\nIt also "

                  // sync: hasAlphaChannelMessage
                  "contains translucency which is not "
                  "fully supported. "
                  "The translucency data will be "
                  "approximated with a 1-bit transparency mask.")
                .arg (wali.itemName ())
                .arg (screenDepthNeeded),
            QString::null,  // or would you prefer "Low Screen Depth and Image Contains Transparency"? :)
            colorDepthTranslucencyDontAskAgain);

        if (!KMessageBox::shouldBeShownContinue (colorDepthTranslucencyDontAskAgain))
        {
            KMessageBox::saveDontShowAgainContinue (colorDepthDontAskAgain);
            KMessageBox::saveDontShowAgainContinue (translucencyDontAskAgain);
        }
    }
    else if (moreColorsThanDisplay)
    {
        KMessageBox::information (wali.parent (),
            i18n ("The %1 "
                  // sync: moreColorsThanDisplayMessage
                  "may have more colors than the current screen mode. "
                  "In order to display it, some colors may be changed. "
                  "Try increasing your screen "
                  "depth to at least %2bpp.")
                .arg (wali.itemName ())
                .arg (screenDepthNeeded),
            i18n ("Low Screen Depth"),
            colorDepthDontAskAgain);
    }
    else if (hasAlphaChannel)
    {
        KMessageBox::information (wali.parent (),
            i18n ("The %1 "
                  // sync: hasAlphaChannelMessage
                  "contains translucency which is not "
                  "fully supported. "
                  "The translucency data will be "
                  "approximated with a 1-bit transparency mask.")
                .arg (wali.itemName ()),
            i18n ("Image Contains Translucency"),
            translucencyDontAskAgain);
    }

    QApplication::restoreOverrideCursor ();
}

// public static
QPixmap kpPixmapFX::convertToPixmap (const QImage &image, bool pretty,
                                     const WarnAboutLossInfo &wali)
{
#if DEBUG_KP_PIXMAP_FX && 1
    kdDebug () << "kpPixmapFX::convertToPixmap(image,pretty=" << pretty
               << ",warnAboutLossInfo.isValid=" << wali.isValid ()
               << ")" << endl;
    QTime timer;
    timer.start ();
#endif

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

#if DEBUG_KP_PIXMAP_FX && 1
    kdDebug () << "\tconversion took " << timer.elapsed () << "msec" << endl;
#endif

    kpPixmapFX::ensureNoAlphaChannel (&destPixmap);


    if (wali.isValid ())
        convertToPixmapWarnAboutLoss (image, wali);


    return destPixmap;
}


//
// Get/Set Parts of Pixmap
//


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

    const QRect validSrcRect = pm.rect ().intersect (rect);
    const bool wouldHaveUndefinedPixels = (validSrcRect != rect);

    if (wouldHaveUndefinedPixels)
    {
    #if DEBUG_KP_PIXMAP_FX && 0
        kdDebug () << "\tret would contain undefined pixels - setting them to transparent" << endl;
    #endif
        QBitmap transparentMask (rect.width (), rect.height ());
        transparentMask.fill (Qt::color0/*transparent*/);
        retPixmap.setMask (transparentMask);
    }

    if (validSrcRect.isEmpty ())
    {
    #if DEBUG_KP_PIXMAP_FX && 0
        kdDebug () << "\tsilly case - completely invalid rect - ret transparent pixmap" << endl;
    #endif
        return retPixmap;
    }


    const QPoint destTopLeft = validSrcRect.topLeft () - rect.topLeft ();

    // copy data _and_ mask (if avail)
    copyBlt (&retPixmap, /* dest */
             destTopLeft.x (), destTopLeft.y (), /* dest pt */
             &pm, /* src */
             validSrcRect.x (), validSrcRect.y (), /* src pt */
             validSrcRect.width (), validSrcRect.height ());

    if (wouldHaveUndefinedPixels && retPixmap.mask () && !pm.mask ())
    {
    #if DEBUG_KP_PIXMAP_FX && 0
        kdDebug () << "\tensure opaque in valid region" << endl;
    #endif
        kpPixmapFX::ensureOpaqueAt (&retPixmap,
                                    QRect (destTopLeft.x (), destTopLeft.y (),
                                           validSrcRect.width (), validSrcRect.height ()));
    }

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
void kpPixmapFX::setPixmapAt (QPixmap *destPixmapPtr, int destX, int destY,
                              const QPixmap &srcPixmap)
{
    kpPixmapFX::setPixmapAt (destPixmapPtr, QPoint (destX, destY), srcPixmap);
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
void kpPixmapFX::paintPixmapAt (QPixmap *destPixmapPtr, int destX, int destY,
                                const QPixmap &srcPixmap)
{
    kpPixmapFX::paintPixmapAt (destPixmapPtr, QPoint (destX, destY), srcPixmap);
}


// public static
kpColor kpPixmapFX::getColorAtPixel (const QPixmap &pm, const QPoint &at)
{
#if DEBUG_KP_PIXMAP_FX && 0
    kdDebug () << "kpToolColorPicker::colorAtPixel" << p << endl;
#endif

    QPixmap pixmap = getPixmapAt (pm, QRect (at, at));
    QImage image = kpPixmapFX::convertToImage (pixmap);
    if (image.isNull ())
    {
        kdError () << "kpPixmapFX::getColorAtPixel(QPixmap) could not convert to QImage" << endl;
        return kpColor::invalid;
    }

    return getColorAtPixel (image, QPoint (0, 0));
}

// public static
kpColor kpPixmapFX::getColorAtPixel (const QPixmap &pm, int x, int y)
{
    return kpPixmapFX::getColorAtPixel (pm, QPoint (x, y));
}

// public static
kpColor kpPixmapFX::getColorAtPixel (const QImage &img, const QPoint &at)
{
    QRgb rgba = img.pixel (at.x (), at.y ());
    return kpColor (rgba);
}

// public static
kpColor kpPixmapFX::getColorAtPixel (const QImage &img, int x, int y)
{
    return kpPixmapFX::getColorAtPixel (img, QPoint (x, y));
}


//
// Mask Operations
//


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
void kpPixmapFX::setMaskAt (QPixmap *destPixmapPtr, int destX, int destY,
                            const QBitmap &srcMaskBitmap)
{
    kpPixmapFX::setMaskAt (destPixmapPtr, QPoint (destX, destY), srcMaskBitmap);
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
                                                const QPixmap &brushBitmap)
{
    if (!destPixmapPtr)
        return;

    if (brushBitmap.depth () > 1)
    {
        kdError () << "kpPixmapFX::paintMaskTransparentWidthBrush() passed brushPixmap with depth > 1" << endl;
        return;
    }

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
void kpPixmapFX::paintMaskTransparentWithBrush (QPixmap *destPixmapPtr, int destX, int destY,
                                                const QPixmap &brushBitmap)
{
    kpPixmapFX::paintMaskTransparentWithBrush (destPixmapPtr,
                                               QPoint (destX, destY),
                                               brushBitmap);
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

// public static
void kpPixmapFX::ensureOpaqueAt (QPixmap *destPixmapPtr, int destX, int destY,
                                 const QPixmap &srcPixmap)
{
    kpPixmapFX::ensureOpaqueAt (destPixmapPtr, QPoint (destX, destY), srcPixmap);
}


//
// Effects
//

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

    #if 1  // dither version
        image = image.convertDepth (1/*monochrome*/);
    #else  // below no dither version looks like true B&W but not close to orig
        image = image.convertDepth (1/*monochrome*/,
                                    Qt::MonoOnly |
                                    Qt::ThresholdDither/*no dither*/ |
                                    Qt::ThresholdAlphaDither/*no dither alpha*/ |
                                    Qt::AvoidDither);
    #endif

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
            QPixmap pixmap = kpPixmapFX::convertToPixmap (image, true/*dither*/);

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
                         const kpColor &backgroundColor, bool fillNewAreas)
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

        if (backgroundColor.isOpaque ())
        {
            painter.begin (destPixmapPtr);
            painter.setPen (backgroundColor.toQColor ());
            painter.setBrush (backgroundColor.toQColor ());
        }

        if (backgroundColor.isTransparent () || destPixmapPtr->mask ())
        {
            maskBitmap = kpPixmapFX::getNonNullMask (*destPixmapPtr);
            maskPainter.begin (&maskBitmap);
            maskPainter.setPen (backgroundColor.maskColor ());
            maskPainter.setBrush (backgroundColor.maskColor ());
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
                            const kpColor &backgroundColor, bool fillNewAreas)
{
    QPixmap ret = pm;
    kpPixmapFX::resize (&ret, w, h, backgroundColor, fillNewAreas);
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
#if DEBUG_KP_PIXMAP_FX && 0
    kdDebug () << "kpPixmapFX::scale(oldRect=" << pm.rect ()
               << ",w=" << w
               << ",h=" << h
               << ",pretty=" << pretty
               << ")"
               << endl;
#endif

    if (w == pm.width () && h == pm.height ())
        return pm;

    if (pretty)
    {
        QImage image = kpPixmapFX::convertToImage (pm);

    #if DEBUG_KP_PIXMAP_FX && 0
        kdDebug () << "\tBefore smooth scale:" << endl;
        for (int y = 0; y < image.height (); y++)
        {
            for (int x = 0; x < image.width (); x++)
            {
                fprintf (stderr, " %08X", image.pixel (x, y));
            }
            fprintf (stderr, "\n");
        }
    #endif

        image = image.smoothScale (w, h);

    #if DEBUG_KP_PIXMAP_FX && 0
        kdDebug () << "\tAfter smooth scale:" << endl;
        for (int y = 0; y < image.height (); y++)
        {
            for (int x = 0; x < image.width (); x++)
            {
                fprintf (stderr, " %08X", image.pixel (x, y));
            }
            fprintf (stderr, "\n");
        }
    #endif

        return kpPixmapFX::convertToPixmap (image, false/*let's not smooth it again*/);
    }
    else
    {
        QWMatrix matrix;

        matrix.scale (double (w) / double (pm.width ()),
                      double (h) / double (pm.height ()));

        return pm.xForm (matrix);
    }
}


static QWMatrix matrixWithZeroOrigin (const QWMatrix &matrix, int width, int height)
{
    QRect newRect = matrix.mapRect (QRect (0, 0, width, height));

    QWMatrix translatedMatrix (matrix.m11 (), matrix.m12 (), matrix.m21 (), matrix.m22 (),
                               matrix.dx () - newRect.left (), matrix.dy () - newRect.top ());

    return translatedMatrix;
}

static QPixmap xForm (const QPixmap &pm, const QWMatrix &transformMatrix_,
                      const kpColor &backgroundColor,
                      int targetWidth, int targetHeight)
{
    QWMatrix transformMatrix = transformMatrix_;

#if DEBUG_KP_PIXMAP_FX && 1
    kdDebug () << "kppixmapfx.cpp: xForm(pm.size=" << pm.size ()
               << ",targetWidth=" << targetWidth
               << ",targetHeight=" << targetHeight
               << ")"
               << endl;
#endif
    QRect newRect = transformMatrix.map (pm.rect ());
#if DEBUG_KP_PIXMAP_FX && 1
    kdDebug () << "\tmappedRect=" << newRect << endl;

#endif

    QWMatrix scaleMatrix;
    if (targetWidth > 0 && targetWidth != newRect.width ())
    {
    #if DEBUG_KP_PIXMAP_FX && 1
        kdDebug () << "\tadjusting for targetWidth" << endl;
    #endif
        scaleMatrix.scale (double (targetWidth) / double (newRect.width ()), 1);
    }

    if (targetHeight > 0 && targetHeight != newRect.height ())
    {
    #if DEBUG_KP_PIXMAP_FX && 1
        kdDebug () << "\tadjusting for targetHeight" << endl;
    #endif
        scaleMatrix.scale (1, double (targetHeight) / double (newRect.height ()));
    }

    if (!scaleMatrix.isIdentity ())
    {
    #if DEBUG_KP_PIXMAP_FX && 1
        // TODO: What is going on here???  Why isn't matrix * working properly?
        QWMatrix wrongMatrix = transformMatrix * scaleMatrix;
        QWMatrix oldHat = transformMatrix;
        if (targetWidth > 0 && targetWidth != newRect.width ())
            oldHat.scale (double (targetWidth) / double (newRect.width ()), 1);
        if (targetHeight > 0 && targetHeight != newRect.height ())
            oldHat.scale (1, double (targetHeight) / double (newRect.height ()));
        QWMatrix altHat = transformMatrix;
        altHat.scale ((targetWidth > 0 && targetWidth != newRect.width ()) ? double (targetWidth) / double (newRect.width ()) : 1,
                      (targetHeight > 0 && targetHeight != newRect.height ()) ? double (targetHeight) / double (newRect.height ()) : 1);
        QWMatrix correctMatrix = scaleMatrix * transformMatrix;

        kdDebug () << "\tsupposedlyWrongMatrix: m11=" << wrongMatrix.m11 ()  // <<<---- this is the correct matrix???
                   << " m12=" << wrongMatrix.m12 ()
                   << " m21=" << wrongMatrix.m21 ()
                   << " m22=" << wrongMatrix.m22 ()
                   << " dx=" << wrongMatrix.dx ()
                   << " dy=" << wrongMatrix.dy ()
                   << " rect=" << wrongMatrix.map (pm.rect ())
                   << endl
                   << "\ti_used_to_use_thisMatrix: m11=" << oldHat.m11 ()
                   << " m12=" << oldHat.m12 ()
                   << " m21=" << oldHat.m21 ()
                   << " m22=" << oldHat.m22 ()
                   << " dx=" << oldHat.dx ()
                   << " dy=" << oldHat.dy ()
                   << " rect=" << oldHat.map (pm.rect ())
                   << endl
                   << "\tabove but scaled at the same time: m11=" << altHat.m11 ()
                   << " m12=" << altHat.m12 ()
                   << " m21=" << altHat.m21 ()
                   << " m22=" << altHat.m22 ()
                   << " dx=" << altHat.dx ()
                   << " dy=" << altHat.dy ()
                   << " rect=" << altHat.map (pm.rect ())
                   << endl
                   << "\tsupposedlyCorrectMatrix: m11=" << correctMatrix.m11 ()
                   << " m12=" << correctMatrix.m12 ()
                   << " m21=" << correctMatrix.m21 ()
                   << " m22=" << correctMatrix.m22 ()
                   << " dx=" << correctMatrix.dx ()
                   << " dy=" << correctMatrix.dy ()
                   << " rect=" << correctMatrix.map (pm.rect ())
                   << endl;
    #endif

        transformMatrix = transformMatrix * scaleMatrix;

        newRect = transformMatrix.map (pm.rect ());
    #if DEBUG_KP_PIXMAP_FX && 1
        kdDebug () << "\tnewRect after targetWidth,targetHeight adjust=" << newRect << endl;
    #endif
    }


    QPixmap newPixmap (targetWidth > 0 ? targetWidth : newRect.width (),
                       targetHeight > 0 ? targetHeight : newRect.height ());
    if ((targetWidth > 0 && targetWidth != newRect.width ()) ||
        (targetHeight > 0 && targetHeight != newRect.height ()))
    {
    #if DEBUG_KP_PIXMAP_FX && 1
        kdDebug () << "kppixmapfx.cpp: xForm(pm.size=" << pm.size ()
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

    if (backgroundColor.isTransparent () || pm.mask ())
    {
        newBitmapMask.resize (newPixmap.width (), newPixmap.height ());
        newBitmapMask.fill (backgroundColor.maskColor ());
    }

    QPainter painter (&newPixmap);
#if DEBUG_KP_PIXMAP_FX && 0
    kdDebug () << "\tmatrix: m11=" << transformMatrix.m11 ()
            << " m12=" << transformMatrix.m12 ()
            << " m21=" << transformMatrix.m21 ()
            << " m22=" << transformMatrix.m22 ()
            << " dx=" << transformMatrix.dx ()
            << " dy=" << transformMatrix.dy ()
            << endl;
#endif
    painter.setWorldMatrix (transformMatrix);
#if DEBUG_KP_PIXMAP_FX && 0
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
        maskPainter.setWorldMatrix (transformMatrix);
        maskPainter.drawPixmap (QPoint (0, 0), kpPixmapFX::getNonNullMask (pm));
        maskPainter.end ();
        newPixmap.setMask (newBitmapMask);
    }

    return newPixmap;
}

// public static
QWMatrix kpPixmapFX::skewMatrix (int width, int height, double hangle, double vangle)
{
    if (hangle == 0 && vangle == 0)
        return QWMatrix ();


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

    return matrixWithZeroOrigin (matrix, width, height);
}

// public static
QWMatrix kpPixmapFX::skewMatrix (const QPixmap &pixmap, double hangle, double vangle)
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
    kdDebug () << "kpPixmapFX::skew() pm.width=" << pm.width ()
               << " pm.height=" << pm.height ()
               << " hangle=" << hangle
               << " vangle=" << vangle
               << " targetWidth=" << targetWidth
               << " targetHeight=" << targetHeight
               << endl;
#endif

    if (hangle == 0 && vangle == 0 && targetWidth <= 0 && targetHeight <= 0)
        return pm;

    // make sure -90 < hangle/vangle < 90 degrees:
    // if (abs (hangle) >= 90 || abs (vangle) >= 90) {
    // TODO: inconsistent
    if (90 - fabs (hangle) < KP_EPSILON || 90 - fabs (vangle) < KP_EPSILON)
    {
        kdError () << "kpPixmapFX::skew() passed hangle and/or vangle out of range (-90 < x < 90)" << endl;
        return pm;
    }


    QWMatrix matrix = skewMatrix (pm, hangle, vangle);

    return ::xForm (pm, matrix, backgroundColor, targetWidth, targetHeight);
}


// public static
QWMatrix kpPixmapFX::rotateMatrix (int width, int height, double angle)
{
    QWMatrix matrix;
    matrix.translate (width / 2, height / 2);
    matrix.rotate (angle);

    return matrixWithZeroOrigin (matrix, width, height);
}

// public static
QWMatrix kpPixmapFX::rotateMatrix (const QPixmap &pixmap, double angle)
{
    return kpPixmapFX::rotateMatrix (pixmap.width (), pixmap.height (), angle);
}


// public static
bool kpPixmapFX::isLosslessRotation (double angle)
{
    // TODO: we shouldn't round to int
    return (qRound (angle) % 90 == 0);
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
    // TODO: epsilon
    if (angle == 0)
        return pm;


    QWMatrix matrix = rotateMatrix (pm, angle);

    return ::xForm (pm, matrix, backgroundColor, targetWidth, targetHeight);
}


// public static
void kpPixmapFX::fill (QPixmap *destPixmapPtr, const kpColor &color)
{
    if (!destPixmapPtr)
        return;

    if (color.isOpaque ())
    {
        destPixmapPtr->setMask (QBitmap ());  // no mask = opaque
        destPixmapPtr->fill (color.toQColor ());
    }
    else
    {
        kpPixmapFX::ensureTransparentAt (destPixmapPtr, destPixmapPtr->rect ());
    }
}

// public static
QPixmap kpPixmapFX::fill (const QPixmap &pm, const kpColor &color)
{
    QPixmap ret = pm;
    kpPixmapFX::fill (&ret, color);
    return ret;
}


// public static
QWMatrix kpPixmapFX::flipMatrix (int width, int height, bool horz, bool vert)
{
    if (width <= 0 || height <= 0)
    {
        kdError () << "kpPixmapFX::flipMatrix() passed invalid dimensions" << endl;
        return QWMatrix ();
    }

    return QWMatrix (horz ? -1 : +1,  // m11
                     0,  // m12
                     0,  // m21
                     vert ? -1 : +1,  // m22
                     horz ? (width - 1) : 0,  // dx
                     vert ? (height - 1) : 0);  // dy
}

// public static
QWMatrix kpPixmapFX::flipMatrix (const QPixmap &pixmap, bool horz, bool vert)
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

    return pm.xForm (flipMatrix (pm, horz, vert));
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
