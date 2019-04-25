/*
   Copyright (c) 2003-2007 Clarence Dang <dang@kde.org>
   Copyright (c) 2017      Martin Koller <kollix@aon.at>
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


#define DEBUG_KP_TOOL_CURVE 0


#include "kpToolCurve.h"
#include "kpLogCategories.h"
#include "environments/tools/kpToolEnvironment.h"
#include "pixmapfx/kpPixmapFX.h"

#include <QPainter>
#include <QPen>
#include <QPainterPath>

#include <KLocalizedString>

//--------------------------------------------------------------------------------

static void DrawCurveShape (kpImage *image,
        const QPolygon &points,
        const kpColor &fcolor, int penWidth,
        const kpColor &bcolor,
        bool isFinal)
{
    (void) bcolor;
    (void) isFinal;

    Q_ASSERT (points.count () >= 2 && points.count () <= 4);

    const QPoint startPoint = points [0];
    const QPoint endPoint = points [1];

    QPoint controlPointP, controlPointQ;

    switch (points.count ())
    {
    // Just a line?
    case 2:
        controlPointP = startPoint;
        controlPointQ = endPoint;
        break;

    // Single control point?
    case 3:
        controlPointP = controlPointQ = points [2];
        break;

    // Two control points?
    case 4:
        controlPointP = points [2];
        controlPointQ = points [3];
        break;
    }

    QPainter painter(image);
    painter.setRenderHint(QPainter::Antialiasing, kpToolEnvironment::drawAntiAliased);

    painter.setPen(QPen(fcolor.toQColor(), penWidth, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));

    if ( kpPixmapFX::Only1PixelInPointArray(points) )
    {
      painter.drawPoint(points[0]);
      return;
    }

    QPainterPath curvePath;
    curvePath.moveTo(startPoint);
    curvePath.cubicTo(controlPointP, controlPointQ, endPoint);

    painter.strokePath(curvePath, painter.pen());
}

//--------------------------------------------------------------------------------

kpToolCurve::kpToolCurve (kpToolEnvironment *environ, QObject *parent)
    : kpToolPolygonalBase (
        i18n ("Curve"),
        i18n ("Draws curves"),
        &::DrawCurveShape,
        Qt::Key_V,
        environ, parent,
        QStringLiteral("tool_curve"))
{
}

kpToolCurve::~kpToolCurve () = default;


// protected virtual [base kpToolPolygonalBase]
QString kpToolCurve::haventBegunShapeUserMessage () const
{
    return i18n ("Drag out the start and end points.");
}


// protected virtual [base kpToolPolygonalBase]
bool kpToolCurve::drawingALine () const
{
    // On the initial drag (consisting of 2 points) creates a line.
    // Future drags are for control points.
    return (points ()->count () == 2);
}


// public virtual [base kpTool]
void kpToolCurve::endDraw (const QPoint &, const QRect &)
{
#if DEBUG_KP_TOOL_CURVE
    qCDebug(kpLogTools) << "kpToolCurve::endDraw()  points="
        << points ()->toList ();
#endif

    switch (points ()->count ())
    {
    // A click of the other mouse button (to finish shape, instead of adding
    // another control point) would have caused endShape() to have been
    // called in kpToolPolygonalBase::beginDraw().  The points list would now
    // be empty.  We are being called by kpTool::mouseReleaseEvent().
    case 0:
        break;

    case 1:
        Q_ASSERT (!"kpToolPolygonalBase::beginDraw() ensures we have >= 2 ctrl points");
        break;

    // Just completed initial line?
    case 2:
        if (originatingMouseButton () == 0)
        {
            setUserMessage (
                i18n ("Left drag to set the first control point or right click to finish."));
        }
        else
        {
            setUserMessage (
                i18n ("Right drag to set the first control point or left click to finish."));
        }

        break;

    // Have initial line and first control point?
    case 3:
        if (originatingMouseButton () == 0)
        {
            setUserMessage (
                i18n ("Left drag to set the last control point or right click to finish."));
        }
        else
        {
            setUserMessage (
                i18n ("Right drag to set the last control point or left click to finish."));
        }

        break;

    // Have initial line and both control points?
    case 4:
    #if DEBUG_KP_TOOL_CURVE
        qCDebug(kpLogTools) << "\tending shape";
    #endif
        endShape ();
        break;

    default:
        Q_ASSERT (!"Impossible number of points");
        break;
    }
}


