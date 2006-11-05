
/*
   Copyright (c) 2003-2006 Clarence Dang <dang@kde.org>
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
#include <qpainterpath.h>
#include <qpixmap.h>
#include <qpoint.h>
#include <qpolygon.h>
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
    kDebug () << "kpPixmapFX::pixmapSize() w=" << width
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
    kDebug () << "kpPixmapFX::imageSize() w=" << width
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
    kDebug () << "kpPixmapFX::stringSize(" << string << ")"
               << " len=" << string.length ()
               << " sizeof(QChar)=" << sizeof (QChar)
               << endl;
#endif
    return string.length () * sizeof (QChar);
}


// public static
int kpPixmapFX::pointArraySize (const QPolygon &points)
{
#if DEBUG_KP_PIXMAP_FX && 1
    kDebug () << "kpPixmapFX::pointArraySize() points.size="
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

    const QImage ret = pixmap.toImage ();
    Q_ASSERT (!ret.isNull ());
    return ret;
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
        wali.m_dontAskAgainPrefix + QLatin1String ("_ColorDepthTranslucency");
    const QString colorDepthDontAskAgain =
        wali.m_dontAskAgainPrefix + QLatin1String ("_ColorDepth");
    const QString translucencyDontAskAgain =
        wali.m_dontAskAgainPrefix + QLatin1String ("_Translucency");

#if DEBUG_KP_PIXMAP_FX && 1
    QTime timer;
    timer.start ();
#endif

    bool hasAlphaChannel =
        (KMessageBox::shouldBeShownContinue (translucencyDontAskAgain) &&
         imageHasAlphaChannel (image));

#if DEBUG_KP_PIXMAP_FX && 1
    kDebug () << "\twarnAboutLoss - check hasAlphaChannel took "
               << timer.restart () << "msec" << endl;
#endif

    bool moreColorsThanDisplay =
        (KMessageBox::shouldBeShownContinue (colorDepthDontAskAgain) &&
         image.depth () > QPixmap::defaultDepth() &&
         QPixmap::defaultDepth () < 24);  // 32 indicates alpha channel

    int screenDepthNeeded = 0;

    if (moreColorsThanDisplay)
        screenDepthNeeded = qMin (24, image.depth ());

#if DEBUG_KP_PIXMAP_FX && 1
    kDebug () << "\ttranslucencyShouldBeShown="
                << KMessageBox::shouldBeShownContinue (translucencyDontAskAgain)
                << endl
                << "\thasAlphaChannel=" << hasAlphaChannel
                << endl
                << "\tcolorDepthShownBeShown="
                << KMessageBox::shouldBeShownContinue (colorDepthDontAskAgain)
                << endl
                << "\timage.depth()=" << image.depth ()
                << endl
                << "\tscreenDepth=" << QPixmap::defaultDepth ()
                << endl
                << "\tmoreColorsThanDisplay=" << moreColorsThanDisplay
                << endl
                << "\tneedDepth=" << screenDepthNeeded
                << endl;
#endif


    QApplication::setOverrideCursor (Qt::ArrowCursor);

    if (moreColorsThanDisplay && hasAlphaChannel)
    {
        KMessageBox::information (wali.m_parent,
            wali.m_moreColorsThanDisplayAndHasAlphaChannelMessage
                .subs (screenDepthNeeded).toString (),
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
                .subs (screenDepthNeeded).toString (),
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
    kDebug () << "kpPixmapFX::convertToPixmap(image,pretty=" << pretty
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
        destPixmap = QPixmap::fromImage(image,
                                     Qt::ColorOnly/*always display depth*/ |
                                     Qt::ThresholdDither/*no dither*/ |
                                     Qt::ThresholdAlphaDither/*no dither alpha*/|
                                     Qt::AvoidDither);
    }
    else
    {
        destPixmap = QPixmap::fromImage (image,
                                     Qt::ColorOnly/*always display depth*/ |
                                     Qt::DiffuseDither/*hi quality dither*/ |
                                     Qt::ThresholdAlphaDither/*no dither alpha*/ |
                                     Qt::PreferDither/*(dither even if <256 colours)*/);
    }

#if DEBUG_KP_PIXMAP_FX && 1
    kDebug () << "\tconversion took " << timer.elapsed () << "msec" << endl;
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
    kDebug () << "kpPixmapFX::convertToPixmapAsLosslessAsPossible(image depth="
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
    Qt::ImageConversionFlags ditherFlags = 0;

    if (image.depth () <= screenDepth)
    {
    #if DEBUG_KP_PIXMAP_FX && 1
        kDebug () << "\timage depth <= screen depth - don't dither"
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
        kDebug () << "\tscreen depth <= 8; imageNumColorsUpTo"
                   << (screenNumColors + 1)
                   << "=" << imageNumColorsUpTo (image, screenNumColors + 1)
                   << endl;
    #endif

        if (imageNumColorsUpTo (image, screenNumColors + 1) <= screenNumColors)
        {
        #if DEBUG_KP_PIXMAP_FX && 1
            kDebug () << "\t\tcolors fit on screen - don't dither"
                       << " (AvoidDither | ThresholdDither)" << endl;
        #endif
            ditherFlags = (Qt::AvoidDither | Qt::ThresholdDither);
        }
        else
        {
        #if DEBUG_KP_PIXMAP_FX && 1
            kDebug () << "\t\tcolors don't fit on screen - dither"
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
        kDebug () << "\tscreen depth > 8 - read config" << endl;
    #endif

        int configDitherIfNumColorsGreaterThan = 323;

        KConfigGroup cfg (KGlobal::config (), kpSettingsGroupGeneral);
        if (cfg.hasKey (kpSettingDitherOnOpen))
        {
            configDitherIfNumColorsGreaterThan = cfg.readEntry (kpSettingDitherOnOpen, 0);
        }
        else
        {
            cfg.writeEntry (kpSettingDitherOnOpen, configDitherIfNumColorsGreaterThan);
            cfg.sync ();
        }

    #if DEBUG_KP_PIXMAP_FX && 1
        kDebug () << "\t\tcfg=" << configDitherIfNumColorsGreaterThan
                   << " image=" << imageNumColorsUpTo (image, configDitherIfNumColorsGreaterThan + 1)
                   << endl;
    #endif

        if (imageNumColorsUpTo (image, configDitherIfNumColorsGreaterThan + 1) >
            configDitherIfNumColorsGreaterThan)
        {
        #if DEBUG_KP_PIXMAP_FX && 1
            kDebug () << "\t\t\talways dither (PreferDither | DiffuseDither)"
                        << endl;
        #endif
            ditherFlags = (Qt::PreferDither | Qt::DiffuseDither);
        }
        else
        {
        #if DEBUG_KP_PIXMAP_FX && 1
            kDebug () << "\t\t\tdon't dither (AvoidDither | ThresholdDither)"
                       << endl;
        #endif
            ditherFlags = (Qt::AvoidDither | Qt::ThresholdDither);
        }
    }


    destPixmap = QPixmap::fromImage (image,
                                 Qt::ColorOnly/*always display depth*/ |
                                 Qt::ThresholdAlphaDither/*no dither alpha*/ |
                                 ditherFlags);

#if DEBUG_KP_PIXMAP_FX && 1
    kDebug () << "\tconversion took " << timer.elapsed () << "msec" << endl;
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
    KP_PFX_CHECK_NO_ALPHA_CHANNEL (pixmap);
    
    if (!pixmap.mask ())
        return pixmap;

    QPixmap retPixmap (pixmap.width (), pixmap.height ());
    retPixmap.fill (transparentColor);

    QPainter p (&retPixmap);
    p.drawPixmap (QPoint (0, 0), pixmap);
    p.end ();

    retPixmap.setMask (pixmap.mask ());
    
    KP_PFX_CHECK_NO_ALPHA_CHANNEL (retPixmap);
    return retPixmap;
}


//
// Drawing Pattern
//

// Warp the given <width> from 1 to 0.
// This is not always done (specifically if <drawingEllipse>) because
// width 0 sometimes looks worse.
//
// Qt lines of width 1 look like they have a width between 1-2 i.e.:
//
// #
//  ##
//   #
//    #
//
// compared to Qt's special "width 0" which just means a "proper" width 1:
//
// #
//  #
//   #
//    #
//
static int WidthToQPenWidth (int width, bool drawingEllipse = false)
{
    if (width == 1)
    {
        // 3x10 ellipse with Qt width 0 looks like rectangle.
        // Therefore, do not apply this 1 -> 0 transformations for ellipses.
        if (!drawingEllipse)
        {
            // Closer to looking width 1, for lines at least.
            return 0;
        }
    }

    return width;
}


// public static
QColor kpPixmapFX::draw_ToQColor (const kpColor &color, bool drawingOnRGBLayer)
{
    if (drawingOnRGBLayer)
    {
        if (color.isOpaque ())
            return color.toQColor ();
        else  // if (color.isTransparent())
        {
            // (arbitrary as image will be transparent here)
            return Qt::black;
        }
    }
    else
        return color.maskColor ();
}


struct DrawPack
{
    QPixmap *image;
    void (*userDrawFunc) (QPainter * /*p*/,
        bool /*drawingOnRGBLayer*/,
        void * /*data*/);
    void *userData;
};

static QRect DrawHelper (QPainter *rgbPainter, QPainter *maskPainter, void *data)
{
    DrawPack *pack = static_cast <DrawPack *> (data);
    
    if (rgbPainter)
        pack->userDrawFunc (rgbPainter, true/*drawing on RGB*/, pack->userData);

    if (maskPainter)
        pack->userDrawFunc (maskPainter, false/*drawing on mask*/, pack->userData);

    // Assume the whole image was clobbered.
    return QRect (0, 0, pack->image->width (), pack->image->height ());
}

// public static
void kpPixmapFX::draw (QPixmap *image,
        void (*drawFunc) (QPainter * /*p*/,
            bool /*drawingOnRGBLayer*/,
            void * /*data*/),
        bool anyColorOpaque, bool anyColorTransparent,
        void *data)
{
    DrawPack pack;
    pack.image = image;
    pack.userDrawFunc = drawFunc;
    pack.userData = data;

    // Call below method.
    kpPixmapFX::draw (image,
        &::DrawHelper,
        anyColorOpaque, anyColorTransparent,
        &pack);
}

// public static
QRect kpPixmapFX::draw (QPixmap *image,
        QRect (*drawFunc) (QPainter * /*rgbPainter*/, QPainter * /*maskPainter*/,
            void * /*data*/),
        bool anyColorOpaque, bool anyColorTransparent,
        void *data)
{
#if DEBUG_KP_PIXMAP_FX
    kDebug () << "kppixmapfx.cpp:Draw(image: rect=" << image->rect ()
              << ",drawFunc=" << drawFunc
              << ",anyColorOpaque=" << anyColorOpaque
              << ",anyColorTransparent=" << anyColorTransparent
              << ")" << endl;
#endif

    KP_PFX_CHECK_NO_ALPHA_CHANNEL (*image);
    
    // Get mask.  Work around the fact that QBitmap's do not have masks.
    // but QBitmap::mask() returns itself.
    QBitmap mask = image->depth () > 1 ?
        image->mask () :
        QBitmap ();

#if DEBUG_KP_PIXMAP_FX
    kDebug () << "\tDraw(): hasMask=" << !mask.isNull () << endl;
#endif
    
    QPainter rgbPainter, maskPainter;
    
    // Draw on RGB layer?
    if (anyColorOpaque)
    {
    #if DEBUG_KP_PIXMAP_FX
        kDebug () << "\tDraw(): drawing on RGB" << endl;
    #endif
        // RGB draw is not allowed to touch mask.
        image->setMask (QBitmap ());

        rgbPainter.begin (image);
    }

    // Draw on mask layer?
    if (anyColorTransparent ||
        !mask.isNull ())
    {
    #if DEBUG_KP_PIXMAP_FX
        kDebug () << "\tDraw(): drawing on transparent" << endl;
    #endif
        if (mask.isNull ())
            mask = kpPixmapFX::getNonNullMask (*image);
            
        maskPainter.begin (&mask);
    }


    if (!rgbPainter.isActive () && !maskPainter.isActive ())
    {
        // We did nothing.
        return QRect ();
    }

        
    const QRect dirtyRect = (*drawFunc) (
        rgbPainter.isActive () ? &rgbPainter : 0,
        maskPainter.isActive () ? &maskPainter : 0,
        data);


    if (rgbPainter.isActive ())
        rgbPainter.end ();

    if (maskPainter.isActive ())
        maskPainter.end ();

    if (anyColorOpaque)
    {
        // A mask should not have been created - that's the job of the next step.
        Q_ASSERT (!image->hasAlpha ());
    }


#if DEBUG_KP_PIXMAP_FX
    kDebug () << "\tDraw(): setting mask " << !mask.isNull () << endl;
#endif

    // Set new mask.
    image->setMask (mask);


    KP_PFX_CHECK_NO_ALPHA_CHANNEL (*image);

    return dirtyRect;
}

// Returns whether there is only 1 distinct point in <points>.
static bool Only1PixelInPointArray (const QPolygon &points)
{
    if (points.count () == 0)
        return false;

    for (int i = 1; i < (int) points.count (); i++)
    {
        if (points [i] != points [0])
            return false;
    }

    return true;
}


//
// Get/Set Parts of Pixmap
//


struct GetSetPaintPixmapAtPack
{
    const QPixmap *srcPixmap;
    QPoint destTopLeft;
    QRect validSrcRect;
    bool isSettingPixelsNotPainting;
};

static void GetSetPaintPixmapAtHelper (QPainter *p, bool drawingOnRGBLayer, void *data)
{
    GetSetPaintPixmapAtPack *pack = static_cast <GetSetPaintPixmapAtPack *> (data);

#if DEBUG_KP_PIXMAP_FX
    kDebug () << "kppixmapfx.cpp:GetSetPaintPixmapAtHelper(drawingOnRGBLayer="
              << drawingOnRGBLayer << ")"
              << "  srcPixmap: rect=" << pack->srcPixmap->size ()
              << " hasAlpha=" << pack->srcPixmap->hasAlpha ()
              << "  destTopLeft=" << pack->destTopLeft
              << " validSrcRect=" << pack->validSrcRect << endl;
#endif

    // Make sure "validSrcRect" lives up to its name.
    Q_ASSERT (pack->validSrcRect.intersect (pack->srcPixmap->rect ()) ==
              pack->validSrcRect);

    if (drawingOnRGBLayer)
    {
        // <srcPixmap> is masked by its mask.
        //
        // For setting-pixels-mode, this doesn't matter since the mask will
        // be copied, hiding the not-copied pixels.
        //
        // For painting-pixels-mode, this is the desired behaviour.
        p->drawPixmap (pack->destTopLeft,
            *pack->srcPixmap,
            pack->validSrcRect);
    }
    else
    {
        const QBitmap srcMask = kpPixmapFX::getNonNullMask (*pack->srcPixmap);
        
        const QRect destRect (pack->destTopLeft.x (), pack->destTopLeft.y (),
                              pack->validSrcRect.width (), pack->validSrcRect.height ());

        // SYNC: Use a Qt "feature": QBitmap's (e.g. "srcMask") are considered to
        //       mask themselves (i.e. "srcMask.mask()" returns "srcMask" instead of
        //       "QBitmap()").  Therefore, "drawPixmap(srcMask)" can never create
        //       transparent pixels.
        //
        //       This is the right behaviour for painting-pixels-mode
        //       (!isSettingPixelsNotPainting) but needs to be worked around for
        //       setting-pixels-mode (isSettingPixelsNotPainting).
        if (pack->isSettingPixelsNotPainting)
        {
            p->fillRect (destRect, Qt::color0/*transparent*/);
        }
         
    // SYNC: HACK around Qt bug:
    //       On non-XRENDER displays, when QPainter is open on a QBitmap,
    //
    //           drawPixmap(point, srcPixmap, QRect (sx, sy, sw, sh))
    //               ["srcPixmap" is also a QBitmap]
    //
    //       ignores (sx,sy) and starts grabbing from srcPixmap at (0,0).
    #if 0
        // This is what we want to write but does not work on non-XRENDER.
        p->drawPixmap (pack->destTopLeft,
            srcMask,
            pack->validSrcRect);
    #else
        // Not needing to think about negatives simplifies the correctness reasoning.
        //
        // This is guaranteed by the above Q_ASSERT() but check anyway in case we
        // accidently remove it.  
        Q_ASSERT (pack->validSrcRect.x () >= 0 && pack->validSrcRect.y () >= 0);

        p->setClipRect (destRect);
        p->drawPixmap (
            QPoint (pack->destTopLeft.x () - pack->validSrcRect.x (),
                    pack->destTopLeft.y () - pack->validSrcRect.y ()),
            srcMask,
            QRect (0,
                   0,
                   pack->validSrcRect.width () + pack->validSrcRect.x (),
                   pack->validSrcRect.height () + pack->validSrcRect.y ()));
    #endif
    }
}

// public static
QPixmap kpPixmapFX::getPixmapAt (const QPixmap &pm, const QRect &rect)
{
    QPixmap retPixmap (rect.width (), rect.height ());

#if DEBUG_KP_PIXMAP_FX && 1
    kDebug () << "kpPixmapFX::getPixmapAt(pm.hasMask="
               << !pm.mask ().isNull ()
               << ",rect="
               << rect
               << ")"
               << endl;
#endif

    KP_PFX_CHECK_NO_ALPHA_CHANNEL (pm);
    
    const QRect validSrcRect = pm.rect ().intersect (rect);
    const bool wouldHaveUndefinedPixels = (validSrcRect != rect);

    // ssss ssss  <-- "pm", "validSrcRect"
    // ssss ssss
    //     +--------+
    // ssss|SSSS    |
    // ssss|SSSS    |  <-- "rect"
    //     |        |
    //     +--------+
    //
    // Let 's' and 'S' be source (pm) pixels.
    //
    // If "rect" asks for part of the source (pm) and some more, the "some
    // more" should be transparent - not undefined.
    if (wouldHaveUndefinedPixels)
    {
    #if DEBUG_KP_PIXMAP_FX && 1
        kDebug () << "\tret would contain undefined pixels - setting them to transparent" << endl;
    #endif
        QBitmap transparentMask (rect.width (), rect.height ());
        transparentMask.fill (Qt::color0/*transparent*/);
        retPixmap.setMask (transparentMask);
    }

    if (validSrcRect.isEmpty ())
    {
    #if DEBUG_KP_PIXMAP_FX && 1
        kDebug () << "\tsilly case - completely invalid rect - ret transparent pixmap" << endl;
    #endif
        return retPixmap;
    }

    // TODO: we could call setPixmapAt().
    GetSetPaintPixmapAtPack pack;
    pack.srcPixmap = &pm;
    pack.destTopLeft = validSrcRect.topLeft () - rect.topLeft ();
    pack.validSrcRect = validSrcRect;
    pack.isSettingPixelsNotPainting = true;

    kpPixmapFX::draw (&retPixmap, &::GetSetPaintPixmapAtHelper,
        true/*always "draw"/copy RGB layer*/,
        (retPixmap.hasAlpha () || pm.hasAlpha ())/*draw on mask if either has one*/,
        &pack);


#if DEBUG_KP_PIXMAP_FX && 1
    kDebug () << "\tretPixmap.hasMask="
               << !retPixmap.mask ().isNull ()
               << endl;
#endif

    KP_PFX_CHECK_NO_ALPHA_CHANNEL (retPixmap);
    return retPixmap;
}


// public static
void kpPixmapFX::setPixmapAt (QPixmap *destPixmapPtr, const QRect &destRect,
                              const QPixmap &srcPixmap)
{
#if DEBUG_KP_PIXMAP_FX && 1
    kDebug () << "kpPixmapFX::setPixmapAt(destPixmap->rect="
               << destPixmapPtr->rect ()
               << ",destPixmap->hasMask="
               << !destPixmapPtr->mask ().isNull ()
               << ",destRect="
               << destRect
               << ",srcPixmap.rect="
               << srcPixmap.rect ()
               << ",srcPixmap.hasMask="
               << !srcPixmap.mask ().isNull ()
               << ")"
               << endl;
#endif

    Q_ASSERT (destPixmapPtr);

    KP_PFX_CHECK_NO_ALPHA_CHANNEL (*destPixmapPtr);
    KP_PFX_CHECK_NO_ALPHA_CHANNEL (srcPixmap);

#if DEBUG_KP_PIXMAP_FX && 0
    if (!destPixmapPtr->mask ().isNull ())
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

        kDebug () << "\tdestPixmapPtr numTrans=" << numTrans << endl;
    }
#endif

    // You cannot copy more than what you have.
    Q_ASSERT (destRect.width () <= srcPixmap.width () &&
              destRect.height () <= srcPixmap.height ());

    GetSetPaintPixmapAtPack pack;
    pack.srcPixmap = &srcPixmap;
    pack.destTopLeft = destRect.topLeft ();
    pack.validSrcRect = QRect (0, 0, destRect.width (), destRect.height ());
    pack.isSettingPixelsNotPainting = true;

    kpPixmapFX::draw (destPixmapPtr, &::GetSetPaintPixmapAtHelper,
        true/*always "draw"/copy RGB layer*/,
        (destPixmapPtr->hasAlpha () || srcPixmap.hasAlpha ())/*draw on mask if either has one*/,
        &pack);

    KP_PFX_CHECK_NO_ALPHA_CHANNEL (*destPixmapPtr);

#if DEBUG_KP_PIXMAP_FX && 0
    kDebug () << "\tdestPixmap->hasMask="
               << !destPixmapPtr->mask ().isNull ()
               << endl;
    if (!destPixmapPtr->mask ().isNull ())
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

        kDebug () << "\tdestPixmapPtr numTrans=" << numTrans << endl;
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
    Q_ASSERT (destPixmapPtr);
    
    KP_PFX_CHECK_NO_ALPHA_CHANNEL (*destPixmapPtr);
    KP_PFX_CHECK_NO_ALPHA_CHANNEL (srcPixmap);

    GetSetPaintPixmapAtPack pack;
    pack.srcPixmap = &srcPixmap;
    pack.destTopLeft = destAt;
    pack.validSrcRect = QRect (0, 0, srcPixmap.width (), srcPixmap.height ());
    pack.isSettingPixelsNotPainting = false;

    kpPixmapFX::draw (destPixmapPtr, &::GetSetPaintPixmapAtHelper,
        true/*always "draw"/copy RGB layer*/,
        destPixmapPtr->hasAlpha ()/*draw on mask only if dest already has one
            (the src will still be masked by its mask if it has one)*/,
        &pack);
    
    KP_PFX_CHECK_NO_ALPHA_CHANNEL (*destPixmapPtr);
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
    kDebug () << "kpToolColorPicker::colorAtPixel" << p << endl;
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
        kError () << "kpPixmapFX::getColorAtPixel(QPixmap) could not convert to QImage" << endl;
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
    {
        // We need to change <destPixmapPtr> from 32-bit (RGBA
        // i.e. hasAlphaChannel() returns true regardless of whether it
        // actually has one, causing trouble later on) to 24-bit
        // (RGB with possible mask).
        //
        // "destPixmapPtr->setMask (destPixmapPtr->mask())" is not
        //  sufficient so do it a long way:

        QPixmap oldPixmap = *destPixmapPtr;

        QBitmap oldMask = oldPixmap.mask ();
        // Kill RGB source mask in case it causes alpha channel / composition
        // operations in the following copy.
        oldPixmap.setMask (QBitmap ());

        // Copy RGB layer.
        *destPixmapPtr = QPixmap (oldPixmap.width (), oldPixmap.height ());
        QPainter p (destPixmapPtr);
        p.drawPixmap (QPoint (0, 0), oldPixmap);
        p.end ();

        // Copy mask layer (if any).
        destPixmapPtr->setMask (oldMask);
    }

    // Note that we don't check this on function entry as the purpose of
    // this function is to force the pixmap to satisfy this invariant.
    KP_PFX_CHECK_NO_ALPHA_CHANNEL (*destPixmapPtr);
}


// public static
QBitmap kpPixmapFX::getNonNullMask (const QPixmap &pm)
{
    KP_PFX_CHECK_NO_ALPHA_CHANNEL (pm);

    const QBitmap mask = pm.mask ()/*slow so cache*/;
    if (!mask.isNull ())
        return mask;
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

    KP_PFX_CHECK_NO_ALPHA_CHANNEL (*destPixmapPtr);
    
    QBitmap maskBitmap = getNonNullMask (*destPixmapPtr);

    QPainter p (&maskBitmap);
    p.fillRect (destRect, Qt::color0/*transparent*/);
    p.end ();

    destPixmapPtr->setMask (maskBitmap);
    
    KP_PFX_CHECK_NO_ALPHA_CHANNEL (*destPixmapPtr);
}


// public static
void kpPixmapFX::paintMaskTransparentWithBrush (QPixmap *destPixmapPtr, const QPoint &destAt,
                                                const QPixmap &brushBitmap)
{
    if (!destPixmapPtr)
        return;

    KP_PFX_CHECK_NO_ALPHA_CHANNEL (*destPixmapPtr);
    
    if (brushBitmap.depth () > 1)
    {
        kError () << "kpPixmapFX::paintMaskTransparentWidthBrush() passed brushPixmap with depth > 1" << endl;
        return;
    }

    if (destPixmapPtr->mask ().isNull ())
        destPixmapPtr->setMask (kpPixmapFX::getNonNullMask (*destPixmapPtr));

    //                  Src
    //  Dest Mask   Brush Bitmap   =   Result
    //  -------------------------------------
    //      0            0               0
    //      0            1               0
    //      1            0               1
    //      1            1               0
    //
    // Dest Bitmap / Result value of 1 = opaque
    //                               0 = transparent
    // Src Brush Bitmap value of 1 means "make transparent"
    //                           0 means "leave it as it is"

    QPixmap brushPixmap (brushBitmap.width (), brushBitmap.height ());
    brushPixmap.fill (Qt::yellow/*arbitrary since source pixels ignored*/);
    brushPixmap.setMask (brushBitmap);  // Mask is not ignored though.

    QPainter painter (destPixmapPtr);
    painter.setCompositionMode (QPainter::CompositionMode_DestinationOut);
    painter.drawPixmap (destAt, brushPixmap);
    painter.end ();
    
    KP_PFX_CHECK_NO_ALPHA_CHANNEL (*destPixmapPtr);
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

    KP_PFX_CHECK_NO_ALPHA_CHANNEL (*destPixmapPtr);

    QBitmap maskBitmap = destPixmapPtr->mask ();

    QPainter p (&maskBitmap);
    p.fillRect (destRect, Qt::color1/*opaque*/);
    p.end ();

    destPixmapPtr->setMask (maskBitmap);
    
    KP_PFX_CHECK_NO_ALPHA_CHANNEL (*destPixmapPtr);
}


//
// Effects
//

// public static
void kpPixmapFX::convertToGrayscale (QPixmap *destPixmapPtr)
{
    KP_PFX_CHECK_NO_ALPHA_CHANNEL (*destPixmapPtr);

    QImage image = kpPixmapFX::convertToImage (*destPixmapPtr);
    kpPixmapFX::convertToGrayscale (&image);
    *destPixmapPtr = kpPixmapFX::convertToPixmap (image);

    KP_PFX_CHECK_NO_ALPHA_CHANNEL (*destPixmapPtr);
}

// public static
QPixmap kpPixmapFX::convertToGrayscale (const QPixmap &pm)
{
    KP_PFX_CHECK_NO_ALPHA_CHANNEL (pm);

    QImage image = kpPixmapFX::convertToImage (pm);
    kpPixmapFX::convertToGrayscale (&image);
    const QPixmap ret = kpPixmapFX::convertToPixmap (image);

    KP_PFX_CHECK_NO_ALPHA_CHANNEL (ret);
    return ret;
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

    KP_PFX_CHECK_NO_ALPHA_CHANNEL (*destPixmapPtr);

    if (color.isOpaque ())
    {
        destPixmapPtr->setMask (QBitmap ());  // no mask = opaque
        destPixmapPtr->fill (color.toQColor ());
    }
    else
    {
        kpPixmapFX::ensureTransparentAt (destPixmapPtr, destPixmapPtr->rect ());
    }
    
    KP_PFX_CHECK_NO_ALPHA_CHANNEL (*destPixmapPtr);
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

    KP_PFX_CHECK_NO_ALPHA_CHANNEL (pm);

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


    KP_PFX_CHECK_NO_ALPHA_CHANNEL (retPixmap);
    return retPixmap;
}


// public static
double kpPixmapFX::AngleInDegreesEpsilon =
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


// public static
QPen kpPixmapFX::QPainterDrawRectPen (const QColor &color, int qtWidth)
{
    return QPen (color, qtWidth, Qt::SolidLine, Qt::SquareCap, Qt::MiterJoin);
}

// public static
QPen kpPixmapFX::QPainterDrawLinePen (const QColor &color, int qtWidth)
{
    return QPen (color, qtWidth, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin);
}


//
// drawPolyline() / drawLine()
//

struct DrawPolylinePackage
{
    QPolygon points;
    kpColor color;
    int penWidth;
};

static void DrawPolylineHelper (QPainter *p,
        bool drawingOnRGBLayer,
        void *data)
{
    DrawPolylinePackage *pack = static_cast <DrawPolylinePackage *> (data);

#if DEBUG_KP_PIXMAP_FX
    kDebug () << "DrawPolylineHelper() points=" << pack->points.toList ()
        << " color=" << (int *) pack->color.toQRgb ()
        << " penWidth=" << pack->penWidth
        << endl;
#endif

    p->setPen (
        kpPixmapFX::QPainterDrawLinePen (
            kpPixmapFX::draw_ToQColor (pack->color, drawingOnRGBLayer),
            ::WidthToQPenWidth (pack->penWidth)));
            
    // Qt bug: single point doesn't show up depending on penWidth.
    if (Only1PixelInPointArray (pack->points))
    {
    #if DEBUG_KP_PIXMAP_FX
        kDebug () << "\tinvoking single point hack" << endl;
    #endif
        p->drawPoint (pack->points [0]);
        return;
    }
    
    p->drawPolyline (pack->points);
}

// public static
void kpPixmapFX::drawPolyline (QPixmap *image,
        const QPolygon &points,
        const kpColor &color, int penWidth)
{
    DrawPolylinePackage pack;
    pack.points = points;
    pack.color = color;
    pack.penWidth = penWidth;
    
    kpPixmapFX::draw (image, &::DrawPolylineHelper,
        color.isOpaque (), color.isTransparent (),
        &pack);
}

// public static
void kpPixmapFX::drawLine (QPixmap *image,
        int x1, int y1, int x2, int y2,
        const kpColor &color, int penWidth)
{
    QPolygon points;
    points.append (QPoint (x1, y1));
    points.append (QPoint (x2, y2));
    
    drawPolyline (image,
        points,
        color, penWidth);
}


//
// drawPolygon()
//

struct DrawPolygonPackage
{
    QPolygon points;
    kpColor fcolor;
    int penWidth;
    kpColor bcolor;
    bool isFinal;
};

static void DrawPolygonHelper (QPainter *p,
        bool drawingOnRGBLayer,
        void *data)
{
    DrawPolygonPackage *pack = static_cast <DrawPolygonPackage *> (data);

#if DEBUG_KP_PIXMAP_FX
    kDebug () << "DrawPolygonHelper() points=" << pack->points.toList ()
        << " fcolor=" << (int *) pack->fcolor.toQRgb ()
        << " penWidth=" << pack->penWidth
        << " bcolor="
        << (int *) (pack->bcolor.isValid () ?
                       pack->bcolor.toQRgb () :
                       0xabadcafe)
        << " isFinal=" << pack->isFinal
        << endl;
#endif

    p->setPen (
        kpPixmapFX::QPainterDrawLinePen (
            kpPixmapFX::draw_ToQColor (pack->fcolor, drawingOnRGBLayer),
            ::WidthToQPenWidth (pack->penWidth)));

    if (pack->bcolor.isValid ())
       p->setBrush (QBrush (kpPixmapFX::draw_ToQColor (pack->bcolor, drawingOnRGBLayer)));
    // HACK: seems to be needed if set_Pen_(Qt::color0) else fills with Qt::color0.
    else
        p->setBrush (Qt::NoBrush);

    // Qt bug: single point doesn't show up depending on penWidth.
    if (Only1PixelInPointArray (pack->points))
    {
    #if DEBUG_KP_PIXMAP_FX
        kDebug () << "\tinvoking single point hack" << endl;
    #endif
        p->drawPoint (pack->points [0]);
        return;
    }

    // TODO: why aren't the ends rounded?
    p->drawPolygon (pack->points, Qt::OddEvenFill);

// TODO
#if 0
    if (pack->isFinal)
        return;
        
    int count = pointsInRect.count ();
    if (count <= 2)
        return;
        
    if (painter.isActive ())
    {
        QPen XORpen = painter.pen ();
        XORpen.setColor (Qt::white);

        painter.setPen (XORpen);
        painter.setRasterOp (Qt::XorROP);
    }

    if (maskPainter.isActive ())
    {
        QPen XORpen = maskPainter.pen ();

        // TODO???
        #if 0
        if (kpTool::isColorTransparent (foregroundColor))
            XORpen.setColor (Qt::color1/*opaque*/);
        else
            XORpen.setColor (Qt::color0/*transparent*/);
        #endif

        maskPainter.setPen (XORpen);
    }

    PAINTER_CALL (drawLine (pointsInRect [0], pointsInRect [count - 1]));
#endif
}

// public static
void kpPixmapFX::drawPolygon (QPixmap *image,
        const QPolygon &points,
        const kpColor &fcolor, int penWidth,
        const kpColor &bcolor,
        bool isFinal)
{
    DrawPolygonPackage pack;
    pack.points = points;
    pack.fcolor = fcolor;
    pack.penWidth = penWidth;
    pack.bcolor = bcolor;
    pack.isFinal = isFinal;
    
    kpPixmapFX::draw (image, &::DrawPolygonHelper,
        fcolor.isOpaque () || (bcolor.isValid () && bcolor.isOpaque ()),
        fcolor.isTransparent () || (bcolor.isValid () && bcolor.isTransparent ()),
        &pack);
}


//
// drawCurve()
//

struct DrawCurvePackage
{
    QPoint startPoint,
        controlPointP, controlPointQ,
        endPoint;
    kpColor color;
    int penWidth;
};

static void DrawCurveHelper (QPainter *p,
        bool drawingOnRGBLayer,
        void *data)
{
    DrawCurvePackage *pack = static_cast <DrawCurvePackage *> (data);

#if DEBUG_KP_PIXMAP_FX
    kDebug () << "DrawCurveHelper() pack: startPoint=" << pack->startPoint
        << " controlPointP=" << pack->controlPointP
        << " controlPointQ=" << pack->controlPointQ
        << " endPoint=" << pack->endPoint
        << " color=" << (int *) pack->color.toQRgb ()
        << " penWidth=" << pack->penWidth
        << endl;
#endif

    const QPen curvePen =
        kpPixmapFX::QPainterDrawLinePen (
            kpPixmapFX::draw_ToQColor (pack->color, drawingOnRGBLayer),
            ::WidthToQPenWidth (pack->penWidth));

    // SYNC: Qt bug: single point doesn't show up depending on penWidth.
    if (pack->startPoint == pack->controlPointP &&
        pack->controlPointP == pack->controlPointQ &&
        pack->controlPointQ == pack->endPoint)
    {
    #if DEBUG_KP_PIXMAP_FX
        kDebug () << "\tinvoking single point hack" << endl;
    #endif
        p->setPen (curvePen);
        p->drawPoint (pack->startPoint);
        return;
    }

    QPainterPath curvePath;
    curvePath.moveTo (pack->startPoint);
    curvePath.cubicTo (pack->controlPointP,
        pack->controlPointQ,
        pack->endPoint);
        
    p->strokePath (curvePath, curvePen);
}

// public static
void kpPixmapFX::drawCurve (QPixmap *image,
    const QPoint &startPoint,
    const QPoint &controlPointP, const QPoint &controlPointQ,
    const QPoint &endPoint,
    const kpColor &color, int penWidth)
{
    DrawCurvePackage pack;
    pack.startPoint = startPoint;
    pack.controlPointP = controlPointP;
    pack.controlPointQ = controlPointQ;
    pack.endPoint = endPoint;
    pack.color = color;
    pack.penWidth = penWidth;
    
    kpPixmapFX::draw (image, &::DrawCurveHelper,
        color.isOpaque (), color.isTransparent (),
        &pack);
}


//
// fillRect()
//

struct FillRectPackage
{
    int x, y, width, height;
    kpColor color;
};

static void FillRectHelper (QPainter *p,
        bool drawingOnRGBLayer,
        void *data)
{
    FillRectPackage *pack = static_cast <FillRectPackage *> (data);

    p->fillRect (pack->x, pack->y, pack->width, pack->height,
        kpPixmapFX::draw_ToQColor (pack->color, drawingOnRGBLayer));
}

// public static
void kpPixmapFX::fillRect (QPixmap *image,
        int x, int y, int width, int height,
        const kpColor &color)
{
    FillRectPackage pack;
    pack.x = x, pack.y = y, pack.width = width, pack.height = height;
    pack.color = color;

    kpPixmapFX::draw (image, &::FillRectHelper,
        color.isOpaque (), color.isTransparent (),
        &pack);
}


//
// DrawGenericRect()
//

struct DrawGenericRectPackage
{
    int x, y, width, height;
    void (*func) (QPainter * /*p*/,
            int /*x*/, int /*y*/,
            int /*width*/, int/*height*/);
    kpColor fcolor;
    int penWidth;
    kpColor bcolor;
    bool isEllipseLike;
};

static void DrawGenericRectHelper (QPainter *p,
        bool drawingOnRGBLayer,
        void *data)
{
    DrawGenericRectPackage *pack = static_cast <DrawGenericRectPackage *> (data);

#if DEBUG_KP_PIXMAP_FX
    kDebug () << "\tkppixmapfx.cpp:DrawGenericRectHelper(drawingOnRGBLayer="
              << drawingOnRGBLayer << ") pack: "
              << pack->x << "," << pack->y << ","
              << pack->width << "," << pack->height << ",func=" << pack->func << ")"
              << " pen.color=" << (int *) pack->fcolor.toQRgb ()
              << " penWidth=" << pack->penWidth
              << " bcolor="
              << (int *) (pack->bcolor.isValid () ?
                             pack->bcolor.toQRgb () :
                             0xabadcafe)
              << " isEllipseLike=" << pack->isEllipseLike
              << endl;
#endif

    p->setPen (
        kpPixmapFX::QPainterDrawRectPen (
            kpPixmapFX::draw_ToQColor (pack->fcolor, drawingOnRGBLayer),
            ::WidthToQPenWidth (pack->penWidth, pack->isEllipseLike)));

    if (pack->bcolor.isValid ())
        p->setBrush (QBrush (kpPixmapFX::draw_ToQColor (pack->bcolor, drawingOnRGBLayer)));
    // HACK: seems to be needed if set_Pen_(Qt::color0) else fills with Qt::color0.
    else
        p->setBrush (Qt::NoBrush);
    
    // Fight Qt behaviour of painting width = fill width + pen width
    // and height = fill height + pen height.  Get rid of pen width.
    (*pack->func) (p,
        pack->x + pack->penWidth / 2,
        pack->y + pack->penWidth / 2,
        pack->width - pack->penWidth,
        pack->height - pack->penWidth);
}

// Calls to drawRect(), drawRoundedRect() and drawEllipse() are
// forwarded here.  <func> is the respective QPainter function and
// may or may not be called.
static void DrawGenericRect (QPixmap *image,
        int x, int y, int width, int height,
        void (*func) (QPainter * /*p*/, int /*x*/, int /*y*/,
                int /*width*/, int/*height*/),
        const kpColor &fcolor, int penWidth,
        const kpColor &bcolor,
        bool isEllipseLike = false)
{
#if DEBUG_KP_PIXMAP_FX
    kDebug () << "kppixmapfx.cpp:DrawGenericRect(" << x << "," << y << ","
        << width << "," << height << ",func=" << func << ")"
        << " pen.color=" << (int *) fcolor.toQRgb ()
        << " penWidth=" << penWidth
        << " bcolor="
        << (int *) (bcolor.isValid () ?
                       bcolor.toQRgb () :
                       0xabadcafe)
        << " isEllipseLike=" << isEllipseLike
        << endl;
 #endif

        
    Q_ASSERT (width > 0);
    Q_ASSERT (height > 0);

    Q_ASSERT (func);

    // Check foreground colour valid.
    // Background is allowed to be invalid (no fill).
    Q_ASSERT (fcolor.isValid ());


    if (width == 1 || height == 1)
    {
    #if DEBUG_KP_PIXMAP_FX
        kDebug () << "\twidth=1 or height=1 - draw line" << endl;
    #endif

        kpPixmapFX::drawLine (image,
            x, y, x + width - 1, y + height - 1,
            fcolor, 1/*force pen width to 1*/);
        return;
    }


    DrawGenericRectPackage pack;
    pack.x = x, pack.y = y, pack.width = width, pack.height = height;
    pack.func = func;
    pack.fcolor = fcolor;
    
    // Outline is so big that fill won't be seen?
    if (penWidth * 2 >= width || penWidth * 2 >= height)
    {
    #if DEBUG_KP_PIXMAP_FX
        kDebug () << "\toutline dominates fill - fill with outline" << endl;
    #endif
                    
        // Fill with outline.
        // TODO: doesn't emulate non-Qt::SolidLine pens
        // TODO: Transition from this hack to normal drawing makes the
        //       ellipse look like it moves 1 pixel to the right due to
        //       Qt missing a pixel on the left of some sizes of ellipses.
        pack.penWidth = 1;
        pack.bcolor = fcolor;  // Outline colour.
    }
    else
    {
        pack.penWidth = penWidth;
        pack.bcolor = bcolor;
    }
    
    pack.isEllipseLike = isEllipseLike;


    kpPixmapFX::draw (image, &::DrawGenericRectHelper,
        fcolor.isOpaque () || (bcolor.isValid () && bcolor.isOpaque ()),
        fcolor.isTransparent () || (bcolor.isValid () && bcolor.isTransparent ()),
        &pack);
}


static void DrawRectHelper (QPainter *p,
        int x, int y, int width, int height)
{
    p->drawRect (x, y, width, height);
}

// public static
void kpPixmapFX::drawRect (QPixmap *image,
        int x, int y, int width, int height,
        const kpColor &fcolor, int penWidth,
        const kpColor &bcolor)
{
    ::DrawGenericRect (image,
        x, y, width, height,
        &::DrawRectHelper,
        fcolor, penWidth,
        bcolor);
}


static void DrawRoundedRectHelper (QPainter *p,
        int x, int y, int width, int height)
{
    // (has default arguments for the roundness i.e. different prototype
    //  to QPainter::draw{Rect,Ellipse}(), therefore need pointer to these
    //  helpers instead of directly to a QPainter member function)
    p->drawRoundRect (x, y, width, height);
}

// public static
void kpPixmapFX::drawRoundedRect (QPixmap *image,
        int x, int y, int width, int height,
        const kpColor &fcolor, int penWidth,
        const kpColor &bcolor)
{
    ::DrawGenericRect (image,
        x, y, width, height,
        &::DrawRoundedRectHelper,
        fcolor, penWidth,
        bcolor,
        true/*ellipse like*/);
}


static void DrawEllipseHelper (QPainter *p,
        int x, int y, int width, int height)
{
    p->drawEllipse (x, y, width, height);
}

// public static
void kpPixmapFX::drawEllipse (QPixmap *image,
        int x, int y, int width, int height,
        const kpColor &fcolor, int penWidth,
        const kpColor &bcolor)
{
    ::DrawGenericRect (image,
        x, y, width, height,
        &::DrawEllipseHelper,
        fcolor, penWidth,
        bcolor,
        true/*ellipse like*/);
}

