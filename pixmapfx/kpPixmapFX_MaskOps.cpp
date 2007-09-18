
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

#ifdef Q_WS_X11
    #include <private/qt_x11_p.h>
#endif


// public static
void kpPixmapFX::initMaskOps ()
{
#if DEBUG_KP_PIXMAP_FX
    kDebug () << "kpPixmapFX::initMaskOps()"
              << "QPixmap::defaultDepth=" << QPixmap::defaultDepth ()
              << "QPixmap().depth()=" << QPixmap ().depth ();
#endif

#ifdef Q_WS_X11
    if (QPixmap::defaultDepth () == 32)
    {
        Q_ASSERT (X11);

        X11->use_xrender = 0;

    #if DEBUG_KP_PIXMAP_FX
        kDebug () << "\tCannot handle alpha channel - disabling XRENDER"
                  << "QPixmap().depth()=" << QPixmap ().depth ();
    #endif
    }
#else
    #warning "KolourPaint is heavily dependent on the behavior of QPixmap under X11."
    #warning "Until KolourPaint gets a proper image library, it is unlikely to work under non-X11."
#endif

    // Check KolourPaint invariant.
    KP_PFX_CHECK_NO_ALPHA_CHANNEL (QPixmap ());
    KP_PFX_CHECK_NO_ALPHA_CHANNEL (QPixmap (1, 1));
    Q_ASSERT (QPixmap ().depth () == QPixmap::defaultDepth ());
    Q_ASSERT (QPixmap (1, 1).depth () == QPixmap::defaultDepth ());


    if (QPixmap::defaultDepth () < 15/*smallest truecolor mode's bpp*/)
    {
        // Even though we support 15-bit truecolor, we're more ambitious and
        // ask for 24-bit since it's safer (see kpPixmapFX::WarnAboutLossInfo).
        KMessageBox::information (0/*parent*/,
            ki18n ("<qt><p>KolourPaint does not support the current screen depth of %1bpp."
                " KolourPaint will attempt to start but may act unreliably.</p>"

                "<p>To avoid this issue, please change your screen depth to 24bpp"
                " and then restart KolourPaint.</p></qt>")
                .subs (QPixmap::defaultDepth ()).toString (),
            i18n ("Unsupported Screen Mode"),
            "startup_unsupported_bpp"/*DontAskAgain ID*/);
    }
}


// public static
bool kpPixmapFX::hasMask (const QPixmap &pixmap)
{
#ifdef Q_WS_X11
    if (QPixmap::defaultDepth () == 32)
    {
        // QPixmap::mask() is hideously slow, and always returns a non-null
        // mask, if the pixmap has an alpha channel (even if the channel is
        // supposed to be empty).
        //
        // Without XRENDER, pixmaps definitely don't have alpha channels.
        // As a result, QPixmap::mask() will be fast and, if there is no mask,
        // it will correctly return a null bitmap.
        Q_ASSERT (!X11->use_xrender);

        // We need this since QPixmap::hasAlpha() lies and returns true
        // purely because the depth is 32.
        //
        // Note: QPixmap::mask() is slightly slow even on a pixmap without an
        //       alpha channel.
        return !pixmap.mask ().isNull ();
    }
#endif

    return pixmap.hasAlpha ();
}


// public static
bool kpPixmapFX::hasAlphaChannel (const QPixmap &pixmap)
{
#ifdef Q_WS_X11
    if (QPixmap::defaultDepth () == 32)
    {
        Q_ASSERT (!X11->use_xrender);

        // We need this path since QPixmap::hasAlphaChannel() lies and returns
        // true purely because the depth is 32.
        //
        // Without XRENDER, pixmaps definitely don't have alpha channels.
        return false;
    }
#endif

    return pixmap.hasAlphaChannel ();
}

// public static
bool kpPixmapFX::checkNoAlphaChannelInternal (const QPixmap &pixmap)
{
    if (!kpPixmapFX::hasAlphaChannel (pixmap))
        return true;

#if 1
    kError () << "Pixmap has alpha channel.  See the .h doc for"
                 " kpPixmapFX::ensureNoAlphaChannel() to fix this.";
    // Ignore bug rather than crashing the program because I bet developers
    // will inadvertently trigger this, when changing the code.  The bug has
    // very annoying effects under XRENDER but only causes serious failure
    // without XRENDER (which is not so common).
    return true;
#else
    // Assert-crash the program.
    return false;
#endif
}


// public static
void kpPixmapFX::ensureNoAlphaChannel (QPixmap *destPixmapPtr)
{
    Q_ASSERT (destPixmapPtr);

    if (kpPixmapFX::hasAlphaChannel (*destPixmapPtr))
    {
        // We need to change <destPixmapPtr> from 32-bit (RGBA
        // i.e. hasAlphaChannel() returns true regardless of whether the
        // channel has any content, causing trouble later on) to 24-bit
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

    // (a bit slow so we cache it)
    const QBitmap mask = pm.mask ();

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
#if DEBUG_KP_PIXMAP_FX
    kDebug () << "kpPixmapFX::paintMaskTransparentWithBrush(destAt="
              << destAt << ") brushRect=" << brushBitmap.rect ()
              << endl;
#endif

    Q_ASSERT (destPixmapPtr);

    KP_PFX_CHECK_NO_ALPHA_CHANNEL (*destPixmapPtr);

    Q_ASSERT (brushBitmap.depth () == 1);

    const QRegion brushRegion = QRegion (brushBitmap).translated (destAt);

    // OPT: Hopelessly inefficent due to function call overhead.
    //      kpPixmapFX should have a function that does this.
    foreach (QRect r, brushRegion.rects ())
    {
    #if DEBUG_KP_PIXMAP_FX
        kDebug () << "\tcopy rect=" << r;
    #endif
        kpPixmapFX::fillRect (destPixmapPtr,
            r.x (), r.y (), r.width (), r.height (),
            kpColor::Transparent);
    }

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
    if (!destPixmapPtr)
        return;

    KP_PFX_CHECK_NO_ALPHA_CHANNEL (*destPixmapPtr);

    // (a bit slow so we cache it)
    QBitmap maskBitmap = destPixmapPtr->mask ();

    // Already opaque?
    if (maskBitmap.isNull ())
        return;

    QPainter p (&maskBitmap);
    p.fillRect (destRect, Qt::color1/*opaque*/);
    p.end ();

    destPixmapPtr->setMask (maskBitmap);

    KP_PFX_CHECK_NO_ALPHA_CHANNEL (*destPixmapPtr);
}

