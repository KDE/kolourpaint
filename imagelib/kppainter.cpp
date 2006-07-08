
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


#define DEBUG_KP_PAINTER 0


#include <kppainter.h>

#include <QBitmap>
#include <QPainter>
#include <QPainterPath>
#include <QPolygon>

#include <kdebug.h>

#include <kpimage.h>
#include <kppixmapfx.h>



// public static
void kpPainter::drawPolyline (kpImage *image,
        const QPolygon &points,
        const kpColor &color, int penWidth)
{
    kpPixmapFX::drawPolyline (image, points, color, penWidth);
}

// public static
void kpPainter::drawLine (kpImage *image,
        int x1, int y1, int x2, int y2,
        const kpColor &color, int penWidth)
{
    kpPixmapFX::drawLine (image, x1, y1, x2, y2, color, penWidth);
}

// public static
void kpPainter::drawPolygon (kpImage *image,
        const QPolygon &points,
        const kpColor &fcolor, int penWidth,
        const kpColor &bcolor,
        bool isFinal)
{
    kpPixmapFX::drawPolygon (image, points, fcolor, penWidth, bcolor, isFinal);
}


// public static
void kpPainter::drawCurve (kpImage *image,
    const QPoint &startPoint,
    const QPoint &controlPointP, const QPoint &controlPointQ,
    const QPoint &endPoint,
    const kpColor &color, int penWidth)
{
    kpPixmapFX::drawCurve (image,
        startPoint, controlPointP, controlPointQ, endPoint, color, penWidth);
}


// public static
void kpPainter::fillRect (kpImage *image,
        int x, int y, int width, int height,
        const kpColor &color)
{
    kpPixmapFX::fillRect (image, x, y, width, height, color);
}


// public static
void kpPainter::drawRect (kpImage *image,
        int x, int y, int width, int height,
        const kpColor &fcolor, int penWidth,
        const kpColor &bcolor)
{
    kpPixmapFX::drawRect (image, x, y, width, height, fcolor, penWidth, bcolor);
}

// public static
void kpPainter::drawRoundedRect (kpImage *image,
        int x, int y, int width, int height,
        const kpColor &fcolor, int penWidth,
        const kpColor &bcolor)
{
    kpPixmapFX::drawRoundedRect (image, x, y, width, height, fcolor, penWidth, bcolor);
}

// public static
void kpPainter::drawEllipse (kpImage *image,
        int x, int y, int width, int height,
        const kpColor &fcolor, int penWidth,
        const kpColor &bcolor)
{
    kpPixmapFX::drawEllipse (image, x, y, width, height, fcolor, penWidth, bcolor);
}
