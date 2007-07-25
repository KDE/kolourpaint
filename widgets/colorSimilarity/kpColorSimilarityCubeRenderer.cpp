
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


#define DEBUG_KP_COLOR_SIMILARITY_CUBE 0


#include <kpColorSimilarityCubeRenderer.h>

#include <math.h>

#include <qpainter.h>
#include <qpixmap.h>
#include <qpolygon.h>

#include <kdebug.h>
#include <klocale.h>

#include <kpColor.h>
#include <kpColorSimilarityHolder.h>
#include <kpDefs.h>


static QColor Color (int redOrGreenOrBlue,
        int baseBrightness,
        double colorSimilarity,
        int similarityDirection,
        int highlight)
{
    int brightness = int (baseBrightness +
                          similarityDirection *
                              .5 * colorSimilarity * kpColorSimilarityHolder::ColorCubeDiagonalDistance);

    if (brightness < 0)
        brightness = 0;
    else if (brightness > 255)
        brightness = 255;

    switch (redOrGreenOrBlue)
    {
    default:
    case 0: return QColor (brightness, highlight, highlight);
    case 1: return QColor (highlight, brightness, highlight);
    case 2: return QColor (highlight, highlight, brightness);
    }
}

static QPoint PointBetween (const QPoint &p, const QPoint &q)
{
    return QPoint ((p.x () + q.x ()) / 2, (p.y () + q.y ()) / 2);
}

static void DrawQuadrant (QPainter *p,
        const QColor &col,
        const QPoint &p1, const QPoint &p2, const QPoint &p3,
        const QPoint pointNotOnOutline)
{
    p->save ();


    QPolygon points (4);
    points [0] = p1;
    points [1] = p2;
    points [2] = p3;
    points [3] = pointNotOnOutline;

    // Polygon fill.
    p->setPen (QPen (col, 0/*neat line of width 1*/));
    p->setBrush (col);
    p->drawPolygon (points);


    // Drawing black outline.
    // TODO: what is "pointNotOnOutline" ???
    points.resize (3);

    p->setPen (QPen (Qt::black, 0/*neat line of width 1*/));
    p->setBrush (Qt::NoBrush);
    p->drawPolyline (points);


    p->restore ();
}

static void DrawFace (QPainter *p,
        double colorSimilarity,
        int redOrGreenOrBlue,
        const QPoint &tl, const QPoint &tr,
        const QPoint &bl, const QPoint &br,
        int highlight)
{
#if DEBUG_KP_COLOR_SIMILARITY_CUBE
    kDebug () << "kpColorSimilarityCubeRenderer.cpp:DrawFace(RorGorB=" << redOrGreenOrBlue
               << ",tl=" << tl
               << ",tr=" << tr
               << ",bl=" << bl
               << ",br=" << br
               << ")"
               << endl;
#endif

    //  tl --- tm --- tr
    //  |      |       |
    //  |      |       |
    //  ml --- mm --- mr
    //  |      |       |
    //  |      |       |
    //  bl --- bm --- br

    const QPoint tm (::PointBetween (tl, tr));
    const QPoint bm (::PointBetween (bl, br));

    const QPoint ml (::PointBetween (tl, bl));
    const QPoint mr (::PointBetween (tr, br));
    const QPoint mm (::PointBetween (ml, mr));


    const int baseBrightness =
        qMax (127,
              255 - int (kpColorSimilarityHolder::MaxColorSimilarity *
                         kpColorSimilarityHolder::ColorCubeDiagonalDistance / 2));
    QColor colors [2] =
    {
        ::Color (redOrGreenOrBlue, baseBrightness, colorSimilarity, -1, highlight),
        ::Color (redOrGreenOrBlue, baseBrightness, colorSimilarity, +1, highlight)
    };

#if DEBUG_KP_COLOR_SIMILARITY_CUBE
    kDebug () << "\tmaxColorSimilarity=" << kpColorSimilarityHolder::MaxColorSimilarity
               << " colorCubeDiagDist=" << kpColorSimilarityHolder::ColorCubeDiagonalDistance
               << endl
               << "\tbaseBrightness=" << baseBrightness
               << " color[0]=" << ((colors [0].rgb () & RGB_MASK) >> ((2 - redOrGreenOrBlue) * 8))
               << " color[1]=" << ((colors [1].rgb () & RGB_MASK) >> ((2 - redOrGreenOrBlue) * 8))
               << endl;
#endif


    ::DrawQuadrant (p, colors [0], tm, tl, ml, mm);
    ::DrawQuadrant (p, colors [1], tm, tr, mr, mm);
    ::DrawQuadrant (p, colors [1], ml, bl, bm, mm);
    ::DrawQuadrant (p, colors [0], bm, br, mr, mm);
}


// public static
void kpColorSimilarityCubeRenderer::Paint (QPainter *p, int size,
        double colorSimilarity,
        int highlight)
{
    Q_ASSERT (highlight >= 0 && highlight <= 255);

    //
    //      P------- Q  ---  ---
    //     /       / |   |    |
    //    /A      /  |  side  |
    //   R-------S   T  --- cubeRectSize
    //   |       |  /   /     |
    // S |       | /  side    |
    //   U-------V   ---     ---
    //   |-------|
    //      side
    //   |-----------|
    //    cubeRectSize
    //
    //

    const double angle = KP_DEGREES_TO_RADIANS (45);
    // S + S sin A = cubeRectSize
    // (1 + sin A) x S = cubeRectSize
    const double side = double (size) / (1 + sin (angle));


    const QPoint pointP ((int) (side * cos (angle)), 0);
    const QPoint pointQ ((int) (side * cos (angle) + side), 0);
    const QPoint pointR (0, (int) (side * sin (angle)));
    const QPoint pointS ((int) (side), (int) (side * sin (angle)));
    const QPoint pointU (0, (int) (side * sin (angle) + side));
    const QPoint pointT ((int) (side + side * cos (angle)), (int) (side));
    const QPoint pointV ((int) (side), (int) (side * sin (angle) + side));


    // Top Face
    ::DrawFace (p,
        colorSimilarity, 0/*red*/,
        pointP, pointQ,
        pointR, pointS,
        highlight);


    // Bottom Face
    ::DrawFace (p,
        colorSimilarity, 1/*green*/,
        pointR, pointS,
        pointU, pointV,
        highlight);


    // Right Face
    ::DrawFace (p,
        colorSimilarity, 2/*blue*/,
        pointS, pointQ,
        pointV, pointT,
        highlight);
}
