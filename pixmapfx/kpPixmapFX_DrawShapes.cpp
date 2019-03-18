
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


#include "kpPixmapFX.h"


#include <QPainter>
#include <QPainterPath>
#include <QImage>
#include <QPoint>
#include <QPolygon>

#include "kpLogCategories.h"

#include "layers/selections/kpAbstractSelection.h"
#include "imagelib/kpColor.h"
#include "kpDefs.h"

//---------------------------------------------------------------------

// Returns whether there is only 1 distinct point in <points>.
bool kpPixmapFX::Only1PixelInPointArray (const QPolygon &points)
{
    if (points.count () == 0) {
        return false;
    }

    for (int i = 1; i < static_cast<int> (points.count ()); i++)
    {
        if (points [i] != points [0]) {
            return false;
        }
    }

    return true;
}

//---------------------------------------------------------------------

// Warp the given <width> from 1 to 0.
// This is not always done (specifically if <drawingEllipse>) because
// width 0 sometimes looks worse.
//
// Qt lines of width 1 look like they have a width between 1-2 i.e.:
//
// #
//  ##
//   #
//    #
//
// compared to Qt's special "width 0" which just means a "proper" width 1:
//
// #
//  #
//   #
//    #
//
static int WidthToQPenWidth (int width, bool drawingEllipse = false)
{
    if (width == 1)
    {
        // 3x10 ellipse with Qt width 0 looks like rectangle.
        // Therefore, do not apply this 1 -> 0 transformations for ellipses.
        if (!drawingEllipse)
        {
            // Closer to looking width 1, for lines at least.
            return 0;
        }
    }

    return width;
}

//---------------------------------------------------------------------

static void QPainterSetPenWithStipple (QPainter *p,
        const kpColor &fColor,
        int penWidth,
        const kpColor &fStippleColor = kpColor::Invalid,
        bool isEllipseLike = false)
{
    if (!fStippleColor.isValid ())
    {
        p->setPen (
           kpPixmapFX::QPainterDrawLinePen (
                fColor.toQColor(),
                ::WidthToQPenWidth (penWidth, isEllipseLike)));
    }
    else
    {
        QPen usePen = kpPixmapFX::QPainterDrawLinePen (
            fColor.toQColor(),
            ::WidthToQPenWidth (penWidth, isEllipseLike));
        usePen.setStyle (Qt::DashLine);
        p->setPen (usePen);

        p->setBackground (fStippleColor.toQColor());
        p->setBackgroundMode (Qt::OpaqueMode);
    }
}

//---------------------------------------------------------------------

// public static
QPen kpPixmapFX::QPainterDrawRectPen (const QColor &color, int qtWidth)
{
    return QPen (color, qtWidth, Qt::SolidLine, Qt::SquareCap, Qt::MiterJoin);
}

//---------------------------------------------------------------------

// public static
QPen kpPixmapFX::QPainterDrawLinePen (const QColor &color, int qtWidth)
{
    return QPen (color, qtWidth, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin);
}

//---------------------------------------------------------------------
//
// drawPolyline() / drawLine()
//

// public static
void kpPixmapFX::drawPolyline (QImage *image,
        const QPolygon &points,
        const kpColor &color, int penWidth,
        const kpColor &stippleColor)
{
    QPainter painter(image);

    ::QPainterSetPenWithStipple(&painter,
        color, penWidth,
        stippleColor);
    
    // Qt bug: single point doesn't show up depending on penWidth.
    if (Only1PixelInPointArray(points))
    {
    #if DEBUG_KP_PIXMAP_FX
        qCDebug(kpLogPixmapfx) << "\tinvoking single point hack";
    #endif
        painter.drawPoint(points[0]);
        return;
    }
    
    painter.drawPolyline(points);
}

//---------------------------------------------------------------------
//
// drawPolygon()
//

// public static
void kpPixmapFX::drawPolygon (QImage *image,
        const QPolygon &points,
        const kpColor &fcolor, int penWidth,
        const kpColor &bcolor,
        bool isFinal,
        const kpColor &fStippleColor)
{
    QPainter p(image);

    ::QPainterSetPenWithStipple (&p,
        fcolor, penWidth,
        fStippleColor);

    if (bcolor.isValid ()) {
        p.setBrush (QBrush (bcolor.toQColor()));
    }
    // HACK: seems to be needed if set_Pen_(Qt::color0) else fills with Qt::color0.
    else {
        p.setBrush (Qt::NoBrush);
    }

    // Qt bug: single point doesn't show up depending on penWidth.
    if (Only1PixelInPointArray (points))
    {
    #if DEBUG_KP_PIXMAP_FX
        qCDebug(kpLogPixmapfx) << "\tinvoking single point hack";
    #endif
        p.drawPoint(points [0]);
        return;
    }

    // TODO: why aren't the ends rounded?
    p.drawPolygon(points, Qt::OddEvenFill);

    if ( isFinal ) {
      return;
    }

    if ( points.count() <= 2 ) {
      return;
    }

    p.setCompositionMode(QPainter::RasterOp_SourceXorDestination);
    p.setPen(QPen(Qt::white));
    p.drawLine(points[0], points[points.count() - 1]);
}

//---------------------------------------------------------------------
// public static

void kpPixmapFX::fillRect (QImage *image,
        int x, int y, int width, int height,
        const kpColor &color,
        const kpColor &stippleColor)
{
    QPainter painter(image);

    if (!stippleColor.isValid ())
    {
        painter.fillRect (x, y, width, height, color.toQColor());
    }
    else
    {
        const int StippleSize = 4;

        painter.setClipRect (x, y, width, height);

        for (int dy = 0; dy < height; dy += StippleSize)
        {
            for (int dx = 0; dx < width; dx += StippleSize)
            {
                const bool parity = ((dy + dx) / StippleSize) % 2;

                kpColor useColor;
                if (!parity) {
                    useColor = color;
                }
                else {
                    useColor = stippleColor;
                }
                    
                painter.fillRect (x + dx, y + dy, StippleSize, StippleSize, useColor.toQColor());
            }
        }

    }
}

//---------------------------------------------------------------------

void kpPixmapFX::drawStippleRect(QImage *image,
        int x, int y, int width, int height,
        const kpColor &fColor,
        const kpColor &fStippleColor)
{
  QPainter painter(image);

  painter.setPen(QPen(fColor.toQColor(), 1, Qt::DashLine));
  painter.setBackground(fStippleColor.toQColor());
  painter.setBackgroundMode(Qt::OpaqueMode);
  painter.drawRect(x, y, width - 1, height - 1);
}

//---------------------------------------------------------------------
