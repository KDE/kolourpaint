
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


#include "kpColorSimilarityCubeRenderer.h"

#include <QtMath>

#include <QPainter>
#include <QPolygonF>

#include "kpLogCategories.h"

#include "widgets/colorSimilarity/kpColorSimilarityHolder.h"
#include "kpDefs.h"

//---------------------------------------------------------------------

static QColor Color (int redOrGreenOrBlue,
        int baseBrightness,
        double colorSimilarity,
        int similarityDirection,
        int highlight)
{
    int brightness = int (baseBrightness +
                          similarityDirection *
                              0.5 * colorSimilarity * kpColorSimilarityHolder::ColorCubeDiagonalDistance);

    if (brightness < 0) {
        brightness = 0;
    }
    else if (brightness > 255) {
        brightness = 255;
    }

    switch (redOrGreenOrBlue)
    {
      default:
      case 0: return  {brightness, highlight, highlight};
      case 1: return  {highlight, brightness, highlight};
      case 2: return  {highlight, highlight, brightness};
    }
}

//---------------------------------------------------------------------

static QPointF PointBetween(const QPointF &p, const QPointF &q)
{
    return {(p.x() + q.x()) / 2.0, (p.y() + q.y()) / 2.0};
}

//---------------------------------------------------------------------

static void DrawQuadrant(QPaintDevice *target,
        const QColor &col,
        const QPointF &p1, const QPointF &p2, const QPointF &p3,
        const QPointF &pointNotOnOutline)
{
    QPolygonF points (4);
    points [0] = p1;
    points [1] = p2;
    points [2] = p3;
    points [3] = pointNotOnOutline;

    QPainter p(target);
    p.setRenderHints(QPainter::Antialiasing, true);

    // Polygon fill.
    p.setPen(col);
    p.setBrush(col);
    p.drawPolygon(points);

    // do not draw a black border. It looks ugly
}

//---------------------------------------------------------------------

static void DrawFace (QPaintDevice *target,
        double colorSimilarity,
        int redOrGreenOrBlue,
        const QPointF &tl, const QPointF &tr,
        const QPointF &bl, const QPointF &br,
        int highlight)
{
#if DEBUG_KP_COLOR_SIMILARITY_CUBE
    qCDebug(kpLogWidgets) << "kpColorSimilarityCubeRenderer.cpp:DrawFace(RorGorB=" << redOrGreenOrBlue
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

    const QPointF tm (::PointBetween (tl, tr));
    const QPointF bm (::PointBetween (bl, br));

    const QPointF ml (::PointBetween (tl, bl));
    const QPointF mr (::PointBetween (tr, br));
    const QPointF mm (::PointBetween (ml, mr));


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
    qCDebug(kpLogWidgets) << "\tmaxColorSimilarity=" << kpColorSimilarityHolder::MaxColorSimilarity
               << " colorCubeDiagDist=" << kpColorSimilarityHolder::ColorCubeDiagonalDistance
               << "\n"
               << "\tbaseBrightness=" << baseBrightness
               << " color[0]=" << ((colors [0].rgba() & RGB_MASK) >> ((2 - redOrGreenOrBlue) * 8))
               << " color[1]=" << ((colors [1].rgba() & RGB_MASK) >> ((2 - redOrGreenOrBlue) * 8))
               << endl;
#endif


    ::DrawQuadrant(target, colors [0], tm, tl, ml, mm);
    ::DrawQuadrant(target, colors [1], tm, tr, mr, mm);
    ::DrawQuadrant(target, colors [1], ml, bl, bm, mm);
    ::DrawQuadrant(target, colors [0], bm, br, mr, mm);
}

//---------------------------------------------------------------------

// public static
void kpColorSimilarityCubeRenderer::Paint(QPaintDevice *target,
        int x, int y, int cubeRectSize,
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

    const double angle = qDegreesToRadians (45.0);
    // S + S sin A = cubeRectSize
    // (1 + sin A) x S = cubeRectSize
    const double side = double(cubeRectSize) / (1.0 + std::sin(angle));


    const QPointF pointP(x + (side * std::cos (angle)),
                         y);
    const QPointF pointQ(x + cubeRectSize - 1,
                         y);
    const QPointF pointR(x,
                         y + (side * std::sin (angle)));
    const QPointF pointS(x + (side),
                         y + (side * std::sin (angle)));
    const QPointF pointT(x + cubeRectSize - 1,
                         y + (side));
    const QPointF pointU(x,
                         y + cubeRectSize - 1);
    const QPointF pointV(x + (side),
                         y + cubeRectSize - 1);


    // Top Face
    ::DrawFace(target,
        colorSimilarity, 0/*red*/,
        pointP, pointQ,
        pointR, pointS,
        highlight);


    // Front Face
    ::DrawFace(target,
        colorSimilarity, 1/*green*/,
        pointR, pointS,
        pointU, pointV,
        highlight);


    // Right Face
    ::DrawFace(target,
        colorSimilarity, 2/*blue*/,
        pointS, pointQ,
        pointV, pointT,
        highlight);
}

//---------------------------------------------------------------------
