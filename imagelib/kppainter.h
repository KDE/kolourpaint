
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


#ifndef KP_PAINTER_H
#define KP_PAINTER_H


#include <kpcolor.h>
#include <kpimage.h>


class QPolygon;


//
// Stateless painter with sane semantics that works on kpImage's i.e. it
// works on document - not view - data.  If you find that you need state,
// you should probably move it into kpPainter to avoid the overhead of
// passing around this state (e.g. color, line width) and for reuse.
//
// Therefore, it should not be used as an on-screen replacement for
// QPainter since kpImage is not supposed to have any relationship with
// QPaintDevice in the future.
//
// This encapsulates the set of functionality used by all of KolourPaint's
// document drawing functions and nothing more, permitting rewriting of
// the graphics backend.  Currently uses QPainter/kpPixmapFX as the backend.
//

struct kpPainterPrivate;

class kpPainter
{
public:
    // Draws a line from (x1,y1) to (x2,y2) onto <image>, with <color>
    // and <width>.  The corners are rounded and centred at those
    // coordinates so if <width> > 1, the line is likely to extend past
    // a rectangle with those corners.
    static void drawPolyline (kpImage *image,
        const QPolygon &points,
        const kpColor &color, int penWidth);
    static void drawLine (kpImage *image,
        int x1, int y1, int x2, int y2,
        const kpColor &color, int penWidth);
    // <isFinal> = shape completed else drawing but haven't finalised.
    // If not <isFinal>, the edge that would form the closure, if the
    // shape were finalised now, is highlighted specially.  Unfortunately,
    // the argument is currently ignored.
    //
    // Odd-even fill.
    static void drawPolygon (kpImage *image,
        const QPolygon &points,
        const kpColor &fcolor, int penWidth,
        const kpColor &bcolor = kpColor::invalid,
        bool isFinal = true);
    // Cubic Beizer.
    static void drawCurve (kpImage *image,
        const QPoint &startPoint,
        const QPoint &controlPointP, const QPoint &controlPointQ,
        const QPoint &endPoint,
        const kpColor &color, int penWidth);

    static void fillRect (kpImage *image,
        int x, int y, int width, int height,
        const kpColor &color);
                
    // Draws a rectangle / rounded rectangle / ellipse with top-left at
    // (x, y) with width <width> and height <height>.  Unlike QPainter,
    // this rectangle will really fit inside <width>x<height> and won't
    // be one pixel higher or wider etc.
    //
    // <width> and <height> must be >= 0.
    //
    // <fcolor> must not be invalid.  However, <bcolor> may be invalid
    // to signify an unfilled rectangle / rounded rectangle /ellipse.
    static void drawRect (kpImage *image,
        int x, int y, int width, int height,
        const kpColor &fcolor, int penWidth = 1,
        const kpColor &bcolor = kpColor::invalid);
    static void drawRoundedRect (kpImage *image,
        int x, int y, int width, int height,
        const kpColor &fcolor, int penWidth = 1,
        const kpColor &bcolor = kpColor::invalid);
    static void drawEllipse (kpImage *image,
        int x, int y, int width, int height,
        const kpColor &fcolor, int penWidth = 1,
        const kpColor &bcolor = kpColor::invalid);
};


#endif  // KP_PAINTER_H
