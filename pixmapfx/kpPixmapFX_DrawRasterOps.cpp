
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
#include <kdebug.h>
#include <kglobal.h>
#include <klocale.h>
#include <kmessagebox.h>

#include <kpAbstractSelection.h>
#include <kpColor.h>
#include <kpDefs.h>
#include <kpTool.h>
#include <kconfiggroup.h>


// public static
void kpPixmapFX::drawStippledXORPolygon (QPixmap *image,
        const QPolygon &points,
        const kpColor &fcolor1, const kpColor &fcolor2,
        const kpColor &color1Hint, const kpColor &color2Hint,
        bool isFinal)
{
   // TODO: Need some XOR simulation.  Trolltech got rid of raster OPs.
    //
    //       On XRENDER, we could do a nice blue with alpha.
    //
    //       But without XRENDER, I vote stippled blue and yellow.  Of course,
    //       Qt 4.2 TP had a bug and stippledness did not work.  Bug should be
    //       fixed now though.
    (void) fcolor1; (void) fcolor2;
    (void) color2Hint;
    if (!isFinal)
    {
        kpPixmapFX::drawPolyline (image,
            points,
            color1Hint, 1/*pen width*/);
    }
    else
    {
        kpPixmapFX::drawPolygon (image,
            points,
            color1Hint, 1/*pen width*/);
    }
}


// public static
void kpPixmapFX::drawStippledXORRect (QPixmap *image,
        int x, int y, int width, int height,
        const kpColor &fcolor1, const kpColor &fcolor2,
        const kpColor &color1Hint, const kpColor &color2Hint)
{
    // TODO: Need some XOR simulation.  Trolltech got rid of raster OPs.
    //
    //       On XRENDER, we could do a nice blue with alpha.
    //
    //       But without XRENDER, I vote stippled blue and yellow.  Of course,
    //       Qt 4.2 TP had a bug and stippledness did not work.  Bug should be
    //       fixed now though.
    (void) fcolor1; (void) fcolor2;
    (void) color2Hint;
    kpPixmapFX::drawRect (image,
        x, y, width, height,
        color1Hint);
}

// public static
void kpPixmapFX::widgetDrawStippledXORRect (QWidget *widget,
        int x, int y, int width, int height,
        const kpColor &fcolor1, const kpColor &fcolor2,
        const kpColor &color1Hint, const kpColor &color2Hint,
        const QRect &clipRect)
{
    // TODO: as per above comment.
    (void) fcolor1; (void) fcolor2;
    (void) color2Hint;

    QPainter p (widget);

    if (!clipRect.isEmpty ())
        p.setClipRect (clipRect);

    p.setPen (color1Hint.toQColor ());

    // TODO: code dup with DrawGenericRect() but hard to not dup
    if (width == 1 || height == 1)
    {
        p.drawLine (x, y, x + width - 1, y + height - 1);
        return;
    }

    p.drawRect (x, y, width - 1, height - 1);
}


// public static
void kpPixmapFX::fillXORRect (QPixmap *image,
        int x, int y, int width, int height,
        const kpColor &fcolor,
        const kpColor &colorHint)
{
    // TODO: XOR simulation or at least, something less solid than current
    //       (in case user's picture contains a lot of the color we're
    //        drawing in).
    (void) fcolor;
    kpPixmapFX::fillRect (image,
        x, y, width, height,
        colorHint);
}

// public static
void kpPixmapFX::widgetFillXORRect (QWidget *widget,
        int x, int y, int width, int height,
        const kpColor &fcolor,
        const kpColor &colorHint)
{
    // TODO: XOR simulation or at least, something less solid than current
    //       (in case user's picture contains a lot of the color we're
    //        drawing in).
    (void) fcolor;
    QPainter p (widget);
    p.fillRect (x, y, width, height, colorHint.toQColor ());
}


// public static
void kpPixmapFX::drawNOTRect (QPixmap *image,
        int x, int y, int width, int height,
        const kpColor &fcolor,
        const kpColor &colorHint)
{
    // TODO: NOT simulation or at least, something less solid than current
    //       (in case user's picture contains a lot of the color we're
    //        drawing in).
    (void) fcolor;
    kpPixmapFX::drawRect (image,
        x, y, width, height,
        colorHint);
}


// public static
void kpPixmapFX::widgetFillNOTRect (QWidget *widget,
        int x, int y, int width, int height,
        const kpColor &fcolor,
        const kpColor &colorHint)
{
    // TODO: NOT simulation or at least, something less solid than current
    //       (in case user's picture contains a lot of the color we're
    //        drawing in).
    (void) fcolor;
    QPainter p (widget);
    p.fillRect (x, y, width, height, colorHint.toQColor ());
}
