
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


#ifndef KP_PAINTER_H
#define KP_PAINTER_H


#include "kpColor.h"
#include "kpImage.h"




//
// Stateless painter with sane semantics that works on kpImage's i.e. it
// works on document - not view - data.  If you find that you need state,
// you should probably move it into kpPainter to avoid the overhead of
// passing around this state (e.g. color, line width) and for reuse.
//
// kpPainter is to kpImage as QPainter is to QPixmap.
//
// This encapsulates the set of functionality used by all of KolourPaint's
// document drawing functions and nothing more, permitting rewriting of
// the image library.  Currently uses QPainter/kpPixmapFX as the image library.
//

struct kpPainterPrivate;

class kpPainter
{
public:
    // helper to make a correct QRect out of 2 QPoints regardless of their relative position
    // to each other
    static QRect normalizedRect(const QPoint& p1, const QPoint& p2)
    {
      return QRect(qMin(p1.x(), p2.x()), qMin(p1.y(), p2.y()),
                   qAbs(p2.x() - p1.x()) + 1, qAbs(p2.y() - p1.y()) + 1);
    }

    // Returns whether the given points are cardinally adjacent (i.e. one point
    // is exactly 1 pixel north, east, south or west of the other).  Equal
    // points are not cardinally adjacent.
    static bool pointsAreCardinallyAdjacent (const QPoint &p, const QPoint &q);

    // Returns a list of points representing a straight line from <startPoint>
    // to <endPoint> inclusive, using Bresenham's line algorithm.  Each point
    // is created only with the specified <probability>.
    //
    // If <cardinalAdjacency> is set, a modified Bresenham's algorithm will add
    // an extra point between every pair of originally strictly-diagonally-adjacent
    // points, such that these points become cardinally adjacent.  However, these
    // extra points are also created only with the specified <probability>.
    //
    // For instance, <cardinalAdjacency> must be set if a diagonal line is to
    // drawn at each of the returned points, otherwise things won't look right:
    //
    //     .\.....
    //     \.\....
    //     .\.B...
    //     ..Ac\..
    //     ...\.\.
    //     ....\..
    //
    // 'A' is the previous Bresenham point.  'B' is the new point.  See how if
    // diagonal lines are drawn at A and B, there is a gap between the lines.
    // Setting <cardinalAdjacency> will solve this problem, since it will add
    // a point at 'c'.
    //
    // ASSUMPTION: <probability> is between 0.0 and 1.0 inclusive.
    static QList <QPoint> interpolatePoints (const QPoint &startPoint,
        const QPoint &endPoint,
        bool cardinalAdjacency = false,
        double probability = 1.0);

    static void fillRect (kpImage *image,
        int x, int y, int width, int height,
        const kpColor &color);

    // Replaces all pixels of <colorToReplace> on the line
    // from (x1,y1) to (x2,y2) of <image>, with a pen of <color> with
    // dimensions <penWidth>x<penHeight>.
    //
    // The corners are centred at those coordinates so if <penWidth> > 1 or
    // <penHeight> > 1, the line is likely to extend past a rectangle with
    // those corners.
    //
    // Returns the dirty rectangle.
    static QRect washLine (kpImage *image,
        int x1, int y1, int x2, int y2,
        const kpColor &color, int penWidth, int penHeight,
        const kpColor &colorToReplace,
        int processedColorSimilarity);

    static QRect washRect (kpImage *image,
        int x, int y, int width, int height,
        const kpColor &color,
        const kpColor &colorToReplace,
        int processedColorSimilarity);

    // For each point in <points>, sprays a random pattern of 10 dots of <color>,
    // each within a circle of diameter <spraycanSize>, onto <image>.
    //
    // ASSUMPTION: spraycanSize > 0.
    // TODO: I think this diameter is 1 or 2 off.
    static void sprayPoints (kpImage *image,
        const QList <QPoint> &points,
        const kpColor &color,
        int spraycanSize);
};


#endif  // KP_PAINTER_H
