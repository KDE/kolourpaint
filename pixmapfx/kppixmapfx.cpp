
/*
   Copyright (c) 2003,2004,2005 Clarence Dang <dang@kde.org>
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
#include <qpointarray.h>
#include <qrect.h>

#include <kconfig.h>
#include <kdebug.h>
#include <klocale.h>
#include <kmessagebox.h>

#include <kpcolor.h>
#include <kpdefs.h>
#include <kpselection.h>
#include <kptool.h>


//
// Overflow Resistant Arithmetic:
//

// public static
int kpPixmapFX::addDimensions (int lhs, int rhs)
{
    if (lhs < 0 || rhs < 0 ||
        lhs > INT_MAX - rhs)
    {
        return INT_MAX;
    }

    return lhs + rhs;
}

// public static
int kpPixmapFX::multiplyDimensions (int lhs, int rhs)
{
    if (rhs == 0)
        return 0;

    if (lhs < 0 || rhs < 0 ||
        lhs > INT_MAX / rhs)
    {
        return INT_MAX;
    }

    return lhs * rhs;
}


//
// QPixmap Statistics
//

// public static
int kpPixmapFX::pixmapArea (const QPixmap &pixmap)
{
    return kpPixmapFX::pixmapArea (pixmap.width (), pixmap.height ());
}

// public static
int kpPixmapFX::pixmapArea (const QPixmap *pixmap)
{
    return (pixmap ? kpPixmapFX::pixmapArea (*pixmap) : 0);
}

// public static
int kpPixmapFX::pixmapArea (int width, int height)
{
    return multiplyDimensions (width, height);
}


// public static
int kpPixmapFX::pixmapSize (const QPixmap &pixmap)
{
    return kpPixmapFX::pixmapSize (pixmap.width (), pixmap.height (),
                                   pixmap.depth ());
}

// public static
int kpPixmapFX::pixmapSize (const QPixmap *pixmap)
{
    return (pixmap ? kpPixmapFX::pixmapSize (*pixmap) : 0);
}

// public static
int kpPixmapFX::pixmapSize (int width, int height, int depth)
{
    // handle 15bpp
    int roundedDepth = (depth > 8 ? (depth + 7) / 8 * 8 : depth);

#if DEBUG_KP_PIXMAP_FX && 0
    kdDebug () << "kpPixmapFX::pixmapSize() w=" << width
               << " h=" << height
               << " d=" << depth
               << " roundedDepth=" << roundedDepth
               << " ret="
               << multiplyDimensions (kpPixmapFX::pixmapArea (width, height), roundedDepth) / 8
               << endl;
#endif
    return multiplyDimensions (kpPixmapFX::pixmapArea (width, height), roundedDepth) / 8;
}


// public static
int kpPixmapFX::imageSize (const QImage &image)
{
    return kpPixmapFX::imageSize (image.width (), image.height (), image.depth ());
}

// public static
int kpPixmapFX::imageSize (const QImage *image)
{
    return (image ? kpPixmapFX::imageSize (*image) : 0);
}

// public static
int kpPixmapFX::imageSize (int width, int height, int depth)
{
    // handle 15bpp
    int roundedDepth = (depth > 8 ? (depth + 7) / 8 * 8 : depth);

#if DEBUG_KP_PIXMAP_FX && 0
    kdDebug () << "kpPixmapFX::imageSize() w=" << width
               << " h=" << height
               << " d=" << depth
               << " roundedDepth=" << roundedDepth
               << " ret="
               << multiplyDimensions (multiplyDimensions (width, height), roundedDepth) / 8
               << endl;
#endif

    return multiplyDimensions (multiplyDimensions (width, height), roundedDepth) / 8;
}


// public static
int kpPixmapFX::selectionSize (const kpSelection &sel)
{
    return sel.size ();
}

// public static
int kpPixmapFX::selectionSize (const kpSelection *sel)
{
    return (sel ? sel->size () : 0);
}


// public static
int kpPixmapFX::stringSize (const QString &string)
{
#if DEBUG_KP_PIXMAP_FX && 1
    kdDebug () << "kpPixmapFX::stringSize(" << string << ")"
               << " len=" << string.length ()
               << " sizeof(QChar)=" << sizeof (QChar)
               << endl;
#endif
    return string.length () * sizeof (QChar);
}


// public static
int kpPixmapFX::pointArraySize (const QPointArray &points)
{
#if DEBUG_KP_PIXMAP_FX && 1
    kdDebug () << "kpPixmapFX::pointArraySize() points.size="
               << points.size ()
               << " sizeof(QPoint)=" << sizeof (QPoint)
               << endl;
#endif

    return (points.size () * sizeof (QPoint));
}


//
// QPixmap/QImage Conversion Functions
//

// public static
QImage kpPixmapFX::convertToImage (const QPixmap &pixmap)
{
    if (pixmap.isNull ())
        return QImage ();

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

static int imageNumColorsUpTo (const QImage &image, int max)
{
    QMap <QRgb, bool> rgbMap;

    if (image.depth () <= 8)
    {
        for (int i = 0; i < image.numColors () && (int) rgbMap.size () < max; i++)
        {
            rgbMap.insert (image.color (i), true);
        }
    }
    else
    {
        for (int y = 0; y < image.height () && (int) rgbMap.size () < max; y++)
        {
            for (int x = 0; x < image.width () && (int) rgbMap.size () < max; x++)
            {
                rgbMap.insert (image.pixel (x, y), true);
            }
        }
    }

    return rgbMap.size ();
}

static void convertToPixmapWarnAboutLoss (const QImage &image,
                                          const kpPixmapFX::WarnAboutLossInfo &wali)
{
    if (!wali.isValid ())
        return;


    const QString colorDepthTranslucencyDontAskAgain =
        wali.m_dontAskAgainPrefix + "_ColorDepthTranslucency";
    const QString colorDepthDontAskAgain =
        wali.m_dontAskAgainPrefix + "_ColorDepth";
    const QString translucencyDontAskAgain =
        wali.m_dontAskAgainPrefix + "_Translucency";

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
        KMessageBox::information (wali.m_parent,
            wali.m_moreColorsThanDisplayAndHasAlphaChannelMessage
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
        KMessageBox::information (wali.m_parent,
            wali.m_moreColorsThanDisplayMessage
                .arg (screenDepthNeeded),
            i18n ("Low Screen Depth"),
            colorDepthDontAskAgain);
    }
    else if (hasAlphaChannel)
    {
        KMessageBox::information (wali.m_parent,
            wali.m_hasAlphaChannelMessage,
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

    if (image.isNull ())
        return QPixmap ();


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
                                     Qt::PreferDither/*(dither even if <256 colours)*/);
    }

#if DEBUG_KP_PIXMAP_FX && 1
    kdDebug () << "\tconversion took " << timer.elapsed () << "msec" << endl;
#endif

    kpPixmapFX::ensureNoAlphaChannel (&destPixmap);


    if (wali.isValid ())
        convertToPixmapWarnAboutLoss (image, wali);


    return destPixmap;
}

// TODO: don't dup convertToPixmap() code
// public static
QPixmap kpPixmapFX::convertToPixmapAsLosslessAsPossible (const QImage &image,
    const WarnAboutLossInfo &wali)
{
#if DEBUG_KP_PIXMAP_FX && 1
    kdDebug () << "kpPixmapFX::convertToPixmapAsLosslessAsPossible(image depth="
               << image.depth ()
               << ",warnAboutLossInfo.isValid=" << wali.isValid ()
               << ") screenDepth=" << QPixmap::defaultDepth ()
               << " imageNumColorsUpTo257=" << imageNumColorsUpTo (image, 257)
               << endl;
    QTime timer;
    timer.start ();
#endif

    if (image.isNull ())
        return QPixmap ();


    const int screenDepth = (QPixmap::defaultDepth () >= 24 ?
                                 32 :
                                 QPixmap::defaultDepth ());

    QPixmap destPixmap;
    int ditherFlags = 0;

    if (image.depth () <= screenDepth)
    {
    #if DEBUG_KP_PIXMAP_FX && 1
        kdDebug () << "\timage depth <= screen depth - don't dither"
                   << " (AvoidDither | ThresholdDither)" << endl;
    #endif

        ditherFlags = (Qt::AvoidDither | Qt::ThresholdDither);
    }
    // PRE: image.depth() > screenDepth
    // ASSERT: screenDepth < 32
    else if (screenDepth <= 8)
    {
        const int screenNumColors = (1 << screenDepth);

    #if DEBUG_KP_PIXMAP_FX && 1
        kdDebug () << "\tscreen depth <= 8; imageNumColorsUpTo"
                   << (screenNumColors + 1)
                   << "=" << imageNumColorsUpTo (image, screenNumColors + 1)
                   << endl;
    #endif

        if (imageNumColorsUpTo (image, screenNumColors + 1) <= screenNumColors)
        {
        #if DEBUG_KP_PIXMAP_FX && 1
            kdDebug () << "\t\tcolors fit on screen - don't dither"
                       << " (AvoidDither | ThresholdDither)" << endl;
        #endif
            ditherFlags = (Qt::AvoidDither | Qt::ThresholdDither);
        }
        else
        {
        #if DEBUG_KP_PIXMAP_FX && 1
            kdDebug () << "\t\tcolors don't fit on screen - dither"
                       << " (PreferDither | DiffuseDither)" << endl;
        #endif
            ditherFlags = (Qt::PreferDither | Qt::DiffuseDither);
        }
    }
    // PRE: image.depth() > screenDepth &&
    //      screenDepth > 8
    // ASSERT: screenDepth < 32
    else
    {
    #if DEBUG_KP_PIXMAP_FX && 1
        kdDebug () << "\tscreen depth > 8 - read config" << endl;
    #endif

        int configDitherIfNumColorsGreaterThan = 323;

        KConfigGroupSaver cfgGroupSaver (KGlobal::config (),
                                         kpSettingsGroupGeneral);
        KConfigBase *cfg = cfgGroupSaver.config ();

        if (cfg->hasKey (kpSettingDitherOnOpen))
        {
            configDitherIfNumColorsGreaterThan = cfg->readNumEntry (kpSettingDitherOnOpen);
        }
        else
        {
            cfg->writeEntry (kpSettingDitherOnOpen, configDitherIfNumColorsGreaterThan);
            cfg->sync ();
        }

    #if DEBUG_KP_PIXMAP_FX && 1
        kdDebug () << "\t\tcfg=" << configDitherIfNumColorsGreaterThan
                   << " image=" << imageNumColorsUpTo (image, configDitherIfNumColorsGreaterThan + 1)
                   << endl;
    #endif

        if (imageNumColorsUpTo (image, configDitherIfNumColorsGreaterThan + 1) >
            configDitherIfNumColorsGreaterThan)
        {
        #if DEBUG_KP_PIXMAP_FX && 1
            kdDebug () << "\t\t\talways dither (PreferDither | DiffuseDither)"
                        << endl;
        #endif
            ditherFlags = (Qt::PreferDither | Qt::DiffuseDither);
        }
        else
        {
        #if DEBUG_KP_PIXMAP_FX && 1
            kdDebug () << "\t\t\tdon't dither (AvoidDither | ThresholdDither)"
                       << endl;
        #endif
            ditherFlags = (Qt::AvoidDither | Qt::ThresholdDither);
        }
    }


    destPixmap.convertFromImage (image,
                                 Qt::ColorOnly/*always display depth*/ |
                                 Qt::ThresholdAlphaDither/*no dither alpha*/ |
                                 ditherFlags);

#if DEBUG_KP_PIXMAP_FX && 1
    kdDebug () << "\tconversion took " << timer.elapsed () << "msec" << endl;
#endif

    kpPixmapFX::ensureNoAlphaChannel (&destPixmap);


    if (wali.isValid ())
        convertToPixmapWarnAboutLoss (image, wali);


    return destPixmap;
}


// public static
QPixmap kpPixmapFX::pixmapWithDefinedTransparentPixels (const QPixmap &pixmap,
                                                        const QColor &transparentColor)
{
    if (!pixmap.mask ())
        return pixmap;

    QPixmap retPixmap (pixmap.width (), pixmap.height ());
    retPixmap.fill (transparentColor);

    QPainter p (&retPixmap);
    p.drawPixmap (QPoint (0, 0), pixmap);
    p.end ();

    retPixmap.setMask (*pixmap.mask ());
    return retPixmap;
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

    // Copy src (masked by src's mask) on top of dest.
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

    if (at.x () < 0 || at.x () >= pm.width () ||
        at.y () < 0 || at.y () >= pm.height ())
    {
        return kpColor::invalid;
    }

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
    if (!img.valid (at.x (), at.y ()))
        return kpColor::invalid;

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


// public static
double kpPixmapFX::AngleInDegreesEpsilon =
    KP_RADIANS_TO_DEGREES (atan (1.0 / 10000.0))
        / (2.0/*max error allowed*/ * 2.0/*for good measure*/);


static QWMatrix matrixWithZeroOrigin (const QWMatrix &matrix, int width, int height)
{
#if DEBUG_KP_PIXMAP_FX
    kdDebug () << "matrixWithZeroOrigin(w=" << width << ",h=" << height << ")" << endl;
    kdDebug () << "\tmatrix: m11=" << matrix.m11 ()
               << " m12=" << matrix.m12 ()
               << " m21=" << matrix.m21 ()
               << " m22=" << matrix.m22 ()
               << " dx=" << matrix.dx ()
               << " dy=" << matrix.dy ()
               << endl;
#endif
    // TODO: Should we be using QWMatrix::Areas?
    QRect newRect = matrix.mapRect (QRect (0, 0, width, height));
#if DEBUG_KP_PIXMAP_FX
    kdDebug () << "\tnewRect=" << newRect << endl;
#endif

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
    // TODO: Should we be using QWMatrix::Areas?
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

        // TODO: Should we be using QWMatrix::Areas?
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
#if DEBUG_KP_PIXMAP_FX && 1
    kdDebug () << "\tmatrix: m11=" << transformMatrix.m11 ()
            << " m12=" << transformMatrix.m12 ()
            << " m21=" << transformMatrix.m21 ()
            << " m22=" << transformMatrix.m22 ()
            << " dx=" << transformMatrix.dx ()
            << " dy=" << transformMatrix.dy ()
            << endl;
    const QWMatrix trueMatrix = QPixmap::trueMatrix (transformMatrix,
        pm.width (), pm.height ());
    kdDebug () << "\ttrue matrix: m11=" << trueMatrix.m11 ()
            << " m12=" << trueMatrix.m12 ()
            << " m21=" << trueMatrix.m21 ()
            << " m22=" << trueMatrix.m22 ()
            << " dx=" << trueMatrix.dx ()
            << " dy=" << trueMatrix.dy ()
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
    if (fabs (hangle - 0) < kpPixmapFX::AngleInDegreesEpsilon &&
        fabs (vangle - 0) < kpPixmapFX::AngleInDegreesEpsilon)
    {
        return QWMatrix ();
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

    if (fabs (hangle - 0) < kpPixmapFX::AngleInDegreesEpsilon &&
        fabs (vangle - 0) < kpPixmapFX::AngleInDegreesEpsilon &&
        (targetWidth <= 0 && targetHeight <= 0)/*don't want to scale?*/)
    {
        return pm;
    }

    if (fabs (hangle) > 90 - kpPixmapFX::AngleInDegreesEpsilon ||
        fabs (vangle) > 90 - kpPixmapFX::AngleInDegreesEpsilon)
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
    if (fabs (angle - 0) < kpPixmapFX::AngleInDegreesEpsilon)
    {
        return QWMatrix ();
    }

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
    const double angleIn = angle;

    // Reflect angle into positive if negative
    if (angle < 0)
        angle = -angle;

    // Remove multiples of 90 to make sure 0 <= angle <= 90
    angle -= ((int) angle) / 90 * 90;

    // "Impossible" situation?
    if (angle < 0 || angle > 90)
    {
        kdError () << "kpPixmapFX::isLosslessRotation(" << angleIn
                   << ") result=" << angle
                   << endl;
        return false;  // better safe than sorry
    }

    const bool ret = (angle < kpPixmapFX::AngleInDegreesEpsilon ||
                      90 - angle < kpPixmapFX::AngleInDegreesEpsilon);
#if DEBUG_KP_PIXMAP_FX
    kdDebug () << "kpPixmapFX::isLosslessRotation(" << angleIn << ")"
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


    QWMatrix matrix = rotateMatrix (pm, angle);

    return ::xForm (pm, matrix, backgroundColor, targetWidth, targetHeight);
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
