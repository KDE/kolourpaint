
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
QImage kpPixmapFX::convertToQImage (const QPixmap &pixmap)
{
    if (pixmap.isNull ())
        return QImage ();

    const QImage ret = pixmap.toImage ();
    Q_ASSERT (!ret.isNull ());
    return ret;
}


// Returns true if <image> contains translucency (rather than just transparency)
// QPixmap::hasAlphaChannel() is a blind method that doesn't actually check
// each pixel.
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
    // OPT: Use hash table, not binary tree.
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

    // TODO: We really need something like KMessageBox::warning() as the
    //       messages are about data loss.

    if (moreColorsThanDisplay && hasAlphaChannel)
    {
        KMessageBox::information (wali.m_parent,
            wali.m_moreColorsThanDisplayAndHasAlphaChannelMessage
                .subs (screenDepthNeeded).toString (),
            i18n ("Loss of Color and Translucency Information"),
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
            i18n ("Loss of Color Information"),
            colorDepthDontAskAgain);
    }
    else if (hasAlphaChannel)
    {
        KMessageBox::information (wali.m_parent,
            wali.m_hasAlphaChannelMessage,
            i18n ("Loss of Translucency Information"),
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
    kDebug () << "\tconversion took " << timer.elapsed () << "msec";
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
        kDebug () << "\tscreen depth > 8 - read config";
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
    kDebug () << "\tconversion took " << timer.elapsed () << "msec";
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

    // Already opaque?
    if (!kpPixmapFX::hasMask (pixmap))
    {
        // No transparent pixels to change RGB of.
        return pixmap;
    }

    QPixmap retPixmap (pixmap.width (), pixmap.height ());
    retPixmap.fill (transparentColor);

    QPainter p (&retPixmap);
    p.drawPixmap (QPoint (0, 0), pixmap);
    p.end ();

    retPixmap.setMask (pixmap.mask ());

    KP_PFX_CHECK_NO_ALPHA_CHANNEL (retPixmap);
    return retPixmap;
}

