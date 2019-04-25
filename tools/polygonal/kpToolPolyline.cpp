
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


#define DEBUG_KP_TOOL_POLYLINE 0


#include "kpToolPolyline.h"
#include "kpLogCategories.h"
#include "environments/tools/kpToolEnvironment.h"
#include "pixmapfx/kpPixmapFX.h"

#include <KLocalizedString>

#include <QPainter>
#include <QPen>

//--------------------------------------------------------------------------------

kpToolPolyline::kpToolPolyline (kpToolEnvironment *environ, QObject *parent)
    : kpToolPolygonalBase (
        i18n ("Connected Lines"),
        i18n ("Draws connected lines"),
        &drawShape,
        Qt::Key_N,
        environ, parent,
        QStringLiteral("tool_polyline"))
{
}

//--------------------------------------------------------------------------------

// private virtual [base kpToolPolygonalBase]
QString kpToolPolyline::haventBegunShapeUserMessage () const
{
    return i18n ("Drag to draw the first line.");
}

//--------------------------------------------------------------------------------
// public static

void kpToolPolyline::drawShape(kpImage *image,
        const QPolygon &points,
        const kpColor &fcolor, int penWidth,
        const kpColor &bcolor,
        bool isFinal)
{
    (void) bcolor;
    (void) isFinal;

  QPainter painter(image);
  painter.setRenderHint(QPainter::Antialiasing, kpToolEnvironment::drawAntiAliased);

  painter.setPen(QPen(fcolor.toQColor(), penWidth, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));

  if ( kpPixmapFX::Only1PixelInPointArray(points) ) {
    painter.drawPoint(points[0]);
  }
  else {
    painter.drawPolyline(points);
  }
}

//--------------------------------------------------------------------------------

// public virtual [base kpTool]
void kpToolPolyline::endDraw (const QPoint &, const QRect &)
{
#if DEBUG_KP_TOOL_POLYLINE
    qCDebug(kpLogTools) << "kpToolPolyline::endDraw()  points="
        << points ()->toList ();
#endif

    // A click of the other mouse button (to finish shape, instead of adding
    // another control point) would have caused endShape() to have been
    // called in kpToolPolygonalBase::beginDraw().  The points list would now
    // be empty.  We are being called by kpTool::mouseReleaseEvent().
    if (points ()->count () == 0) {
        return;
    }

    if (points ()->count () >= kpToolPolygonalBase::MaxPoints)
    {
    #if DEBUG_KP_TOOL_POLYLINE
        qCDebug(kpLogTools) << "\tending shape";
    #endif
        endShape ();
        return;
    }

    if (originatingMouseButton () == 0)
    {
        setUserMessage (i18n ("Left drag another line or right click to finish."));
    }
    else
    {
        setUserMessage (i18n ("Right drag another line or left click to finish."));
    }
}


