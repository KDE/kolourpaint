
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
              << " hasMask=" << kpPixmapFX::hasMask (*pack->srcPixmap)
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

        p->drawPixmap (pack->destTopLeft,
            srcMask,
            pack->validSrcRect);
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
        kDebug () << "\tret would contain undefined pixels - setting them to transparent";
    #endif
        QBitmap transparentMask (rect.width (), rect.height ());
        transparentMask.fill (Qt::color0/*transparent*/);
        retPixmap.setMask (transparentMask);
    }

    if (validSrcRect.isEmpty ())
    {
    #if DEBUG_KP_PIXMAP_FX && 1
        kDebug () << "\tsilly case - completely invalid rect - ret transparent pixmap";
    #endif
        return retPixmap;
    }

    // REFACTOR: we could call setPixmapAt().
    GetSetPaintPixmapAtPack pack;
    pack.srcPixmap = &pm;
    pack.destTopLeft = validSrcRect.topLeft () - rect.topLeft ();
    pack.validSrcRect = validSrcRect;
    pack.isSettingPixelsNotPainting = true;

    kpPixmapFX::draw (&retPixmap, &::GetSetPaintPixmapAtHelper,
        true/*always "draw"/copy RGB layer*/,
        (kpPixmapFX::hasMask (retPixmap) ||
         kpPixmapFX::hasMask (pm))/*draw on mask if either has one*/,
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
        QImage image = kpPixmapFX::convertToQImage (*destPixmapPtr);
        int numTrans = 0;

        for (int y = 0; y < image.height (); y++)
        {
            for (int x = 0; x < image.width (); x++)
            {
                if (qAlpha (image.pixel (x, y)) == 0)
                    numTrans++;
            }
        }

        kDebug () << "\tdestPixmapPtr numTrans=" << numTrans;
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
        (kpPixmapFX::hasMask (*destPixmapPtr) ||
         kpPixmapFX::hasMask (srcPixmap))/*draw on mask if either has one*/,
        &pack);

    KP_PFX_CHECK_NO_ALPHA_CHANNEL (*destPixmapPtr);

#if DEBUG_KP_PIXMAP_FX && 0
    kDebug () << "\tdestPixmap->hasMask="
               << !destPixmapPtr->mask ().isNull ()
               << endl;
    if (!destPixmapPtr->mask ().isNull ())
    {
        QImage image = kpPixmapFX::convertToQImage (*destPixmapPtr);
        int numTrans = 0;

        for (int y = 0; y < image.height (); y++)
        {
            for (int x = 0; x < image.width (); x++)
            {
                if (qAlpha (image.pixel (x, y)) == 0)
                    numTrans++;
            }
        }

        kDebug () << "\tdestPixmapPtr numTrans=" << numTrans;
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
        kpPixmapFX::hasMask (*destPixmapPtr)/*draw on mask only if dest already has one
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
    kDebug () << "kpToolColorPicker::colorAtPixel" << p;
#endif

    if (at.x () < 0 || at.x () >= pm.width () ||
        at.y () < 0 || at.y () >= pm.height ())
    {
        return kpColor::Invalid;
    }

    QPixmap pixmap = getPixmapAt (pm, QRect (at, at));
    QImage image = kpPixmapFX::convertToQImage (pixmap);
    if (image.isNull ())
    {
        kError () << "kpPixmapFX::getColorAtPixel(QPixmap) could not convert to QImage" << endl;
        return kpColor::Invalid;
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
        return kpColor::Invalid;

    QRgb rgba = img.pixel (at.x (), at.y ());
    return kpColor (rgba);
}

// public static
kpColor kpPixmapFX::getColorAtPixel (const QImage &img, int x, int y)
{
    return kpPixmapFX::getColorAtPixel (img, QPoint (x, y));
}
