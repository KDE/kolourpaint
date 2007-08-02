
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

