
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
#if DEBUG_KP_PIXMAP_FX && 1
    #include <qimage.h>
#endif
#include <qpainter.h>
#include <qpixmap.h>
#include <qpoint.h>
#include <qrect.h>

#include <kdebug.h>

#include <kpdefs.h>
#include <kppixmapfx.h>


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
