
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

#include <qpainter.h>
#include <qpainterpath.h>
#include <QImage>
#include <qpoint.h>
#include <qpolygon.h>

#include <kdebug.h>

#include <kpAbstractSelection.h>
#include <kpColor.h>
#include <kpDefs.h>

//---------------------------------------------------------------------

// Returns whether there is only 1 distinct point in <points>.
static bool Only1PixelInPointArray (const QPolygon &points)
{
    if (points.count () == 0)
        return false;

    for (int i = 1; i < (int) points.count (); i++)
    {
        if (points [i] != points [0])
            return false;
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
        kDebug () << "\tinvoking single point hack";
    #endif
        painter.drawPoint(points[0]);
        return;
    }
    
    painter.drawPolyline(points);
}

//---------------------------------------------------------------------

// public static
void kpPixmapFX::drawLine (QImage *image,
        int x1, int y1, int x2, int y2,
        const kpColor &color, int penWidth,
        const kpColor &stippleColor)
{
    QPolygon points;
    points.append (QPoint (x1, y1));
    points.append (QPoint (x2, y2));

    drawPolyline (image,
        points,
        color, penWidth,
        stippleColor);
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

    if (bcolor.isValid ())
        p.setBrush (QBrush (bcolor.toQColor()));
    // HACK: seems to be needed if set_Pen_(Qt::color0) else fills with Qt::color0.
    else
        p.setBrush (Qt::NoBrush);

    // Qt bug: single point doesn't show up depending on penWidth.
    if (Only1PixelInPointArray (points))
    {
    #if DEBUG_KP_PIXMAP_FX
        kDebug () << "\tinvoking single point hack";
    #endif
        p.drawPoint(points [0]);
        return;
    }

    // TODO: why aren't the ends rounded?
    p.drawPolygon(points, Qt::OddEvenFill);

    if ( isFinal )
      return;

    if ( points.count() <= 2 )
      return;

    p.setCompositionMode(QPainter::RasterOp_SourceXorDestination);
    p.setPen(QPen(Qt::white));
    p.drawLine(points[0], points[points.count() - 1]);
}

//---------------------------------------------------------------------

// public static
void kpPixmapFX::drawCurve (QImage *image,
    const QPoint &startPoint,
    const QPoint &controlPointP, const QPoint &controlPointQ,
    const QPoint &endPoint,
    const kpColor &color, int penWidth)
{
    QPainter p(image);
    ::QPainterSetPenWithStipple (&p,
        color, penWidth);

    // SYNC: Qt bug: single point doesn't show up depending on penWidth.
    if (startPoint == controlPointP &&
        controlPointP == controlPointQ &&
        controlPointQ == endPoint)
    {
    #if DEBUG_KP_PIXMAP_FX
        kDebug () << "\tinvoking single point hack";
    #endif
        p.drawPoint (startPoint);
        return;
    }

    QPainterPath curvePath;
    curvePath.moveTo(startPoint);
    curvePath.cubicTo(controlPointP, controlPointQ, endPoint);

    p.strokePath(curvePath, p.pen());
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
                if (!parity)
                    useColor = color;
                else
                    useColor = stippleColor;
                    
                painter.fillRect (x + dx, y + dy, StippleSize, StippleSize, useColor.toQColor());
            }
        }

    }
}

//---------------------------------------------------------------------
// Calls to drawRect(), drawRoundedRect() and drawEllipse() are
// forwarded here.  <func> is the respective QPainter function and
// may or may not be called.
static void DrawGenericRect (QImage *image,
        int x, int y, int width, int height,
        void (*func) (QPainter * /*p*/, int /*x*/, int /*y*/,
                int /*width*/, int/*height*/),
        const kpColor &fcolor, int penWidth,
        kpColor bcolor,
        const kpColor &fStippleColor,
        bool isEllipseLike)
{
#if DEBUG_KP_PIXMAP_FX
    kDebug () << "kppixmapfx.cpp:DrawGenericRect(" << x << "," << y << ","
        << width << "," << height << ",func=" << func << ")"
        << " pen.color=" << (int *) fcolor.toQRgb ()
        << " penWidth=" << penWidth
        << " bcolor="
        << (int *) (bcolor.isValid () ?
                       bcolor.toQRgb () :
                       0xabadcafe)
        << " isEllipseLike=" << isEllipseLike
        << endl;
 #endif


    if ( (width == 0) || (height == 0) )
      return;

    Q_ASSERT (func);

    // Check foreground colour valid.
    // Background is allowed to be invalid (no fill).
    Q_ASSERT (fcolor.isValid ());


    if (width == 1 || height == 1)
    {
    #if DEBUG_KP_PIXMAP_FX
        kDebug () << "\twidth=1 or height=1 - draw line";
    #endif

        kpPixmapFX::drawLine (image,
            x, y, x + width - 1, y + height - 1,
            fcolor, 1/*force pen width to 1*/,
            fStippleColor);
        return;
    }


    // Outline is so big that fill won't be seen?
    if (penWidth * 2 >= width || penWidth * 2 >= height)
    {
    #if DEBUG_KP_PIXMAP_FX
        kDebug () << "\toutline dominates fill - fill with outline";
    #endif

        // Fill with outline.
        // TODO: doesn't emulate non-Qt::SolidLine pens
        // TODO: Transition from this hack to normal drawing makes the
        //       ellipse look like it moves 1 pixel to the right due to
        //       Qt missing a pixel on the left of some sizes of ellipses.
        penWidth = 1;
        bcolor = fcolor;  // Outline colour.
    }

    QPainter painter(image);

    ::QPainterSetPenWithStipple(&painter,
        fcolor, penWidth,
        fStippleColor,
        isEllipseLike);

    QPen pen = painter.pen();
    pen.setJoinStyle(Qt::MiterJoin);  // rectangle shall always have square corners
    painter.setPen(pen);

    if (bcolor.isValid ())
        painter.setBrush (QBrush (bcolor.toQColor()));
    // HACK: seems to be needed if set_Pen_(Qt::color0) else fills with Qt::color0.
    else
        painter.setBrush (Qt::NoBrush);

    // Fight Qt behaviour of painting width = fill width + pen width
    // and height = fill height + pen height.  Get rid of pen width.
    (*func) (&painter,
        x + penWidth / 2,
        y + penWidth / 2,
        width - penWidth,
        height - penWidth);
}

//---------------------------------------------------------------------


static void DrawRectHelper (QPainter *p,
        int x, int y, int width, int height)
{
    p->drawRect (x, y, width, height);
}

//---------------------------------------------------------------------

// public static
void kpPixmapFX::drawRect (QImage *image,
        int x, int y, int width, int height,
        const kpColor &fcolor, int penWidth,
        const kpColor &bcolor,
        const kpColor &fStippleColor)
{
    ::DrawGenericRect (image,
        x, y, width, height,
        &::DrawRectHelper,
        fcolor, penWidth,
        bcolor,
        fStippleColor,
        false/*not ellipse-like*/);
}

//---------------------------------------------------------------------


static void DrawRoundedRectHelper (QPainter *p,
        int x, int y, int width, int height)
{
    // (has default arguments for the roundness i.e. different prototype
    //  to QPainter::draw{Rect,Ellipse}(), therefore need pointer to these
    //  helpers instead of directly to a QPainter member function)
    p->drawRoundedRect(x, y, width, height, 25, 25, Qt::RelativeSize);
}

//---------------------------------------------------------------------

// public static
void kpPixmapFX::drawRoundedRect (QImage *image,
        int x, int y, int width, int height,
        const kpColor &fcolor, int penWidth,
        const kpColor &bcolor,
        const kpColor &fStippleColor)
{
    ::DrawGenericRect (image,
        x, y, width, height,
        &::DrawRoundedRectHelper,
        fcolor, penWidth,
        bcolor,
        fStippleColor,
        true/*ellipse like*/);
}

//---------------------------------------------------------------------


static void DrawEllipseHelper (QPainter *p,
        int x, int y, int width, int height)
{
    p->drawEllipse (x, y, width, height);
}

//---------------------------------------------------------------------

// public static
void kpPixmapFX::drawEllipse (QImage *image,
        int x, int y, int width, int height,
        const kpColor &fcolor, int penWidth,
        const kpColor &bcolor,
        const kpColor &fStippleColor)
{
    ::DrawGenericRect (image,
        x, y, width, height,
        &::DrawEllipseHelper,
        fcolor, penWidth,
        bcolor,
        fStippleColor,
        true/*ellipse like*/);
}

//---------------------------------------------------------------------
