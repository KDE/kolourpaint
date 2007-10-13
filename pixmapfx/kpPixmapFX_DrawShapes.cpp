
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
#include <kconfiggroup.h>
#include <kdebug.h>
#include <kglobal.h>
#include <klocale.h>
#include <kmessagebox.h>

#include <kpAbstractSelection.h>
#include <kpColor.h>
#include <kpDefs.h>
#include <kpTool.h>


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


static void QPainterSetPenWithStipple (QPainter *p,
        bool drawingOnRGBLayer,
        const kpColor &fColor,
        int penWidth,
        const kpColor &fStippleColor = kpColor::Invalid,
        bool isEllipseLike = false)
{
    if (!fStippleColor.isValid ())
    {
        p->setPen (
           kpPixmapFX::QPainterDrawLinePen (
                kpPixmapFX::draw_ToQColor (fColor, drawingOnRGBLayer),
                ::WidthToQPenWidth (penWidth, isEllipseLike)));
    }
    else
    {
        QPen usePen = kpPixmapFX::QPainterDrawLinePen (
            kpPixmapFX::draw_ToQColor (fColor, drawingOnRGBLayer),
            ::WidthToQPenWidth (penWidth, isEllipseLike));
        usePen.setStyle (Qt::DashLine);
        p->setPen (usePen);

        p->setBackground (kpPixmapFX::draw_ToQColor (fStippleColor, drawingOnRGBLayer));
        p->setBackgroundMode (Qt::OpaqueMode);
    }
}


// public static
QPen kpPixmapFX::QPainterDrawRectPen (const QColor &color, int qtWidth)
{
    return QPen (color, qtWidth, Qt::SolidLine, Qt::SquareCap, Qt::MiterJoin);
}

// public static
QPen kpPixmapFX::QPainterDrawLinePen (const QColor &color, int qtWidth)
{
    return QPen (color, qtWidth, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin);
}


//
// drawPolyline() / drawLine()
//

struct DrawPolylinePackage
{
    QPolygon points;
    kpColor color;
    int penWidth;
    kpColor stippleColor;
};

static void DrawPolylineHelper (QPainter *p,
        bool drawingOnRGBLayer,
        void *data)
{
    DrawPolylinePackage *pack = static_cast <DrawPolylinePackage *> (data);

#if DEBUG_KP_PIXMAP_FX
    kDebug () << "DrawPolylineHelper() points=" << pack->points.toList ()
        << " color=" << (int *) pack->color.toQRgb ()
        << " penWidth=" << pack->penWidth
        << endl;
#endif

    ::QPainterSetPenWithStipple (p, drawingOnRGBLayer,
        pack->color, pack->penWidth,
        pack->stippleColor);
    
    // Qt bug: single point doesn't show up depending on penWidth.
    if (Only1PixelInPointArray (pack->points))
    {
    #if DEBUG_KP_PIXMAP_FX
        kDebug () << "\tinvoking single point hack";
    #endif
        p->drawPoint (pack->points [0]);
        return;
    }
    
    p->drawPolyline (pack->points);
}

// public static
void kpPixmapFX::drawPolyline (QPixmap *image,
        const QPolygon &points,
        const kpColor &color, int penWidth,
        const kpColor &stippleColor)
{
    DrawPolylinePackage pack;
    pack.points = points;
    pack.color = color;
    pack.penWidth = penWidth;
    pack.stippleColor = stippleColor;

    kpPixmapFX::draw (image, &::DrawPolylineHelper,
        color.isOpaque () || (stippleColor.isValid () && stippleColor.isOpaque ()),
        color.isTransparent () || (stippleColor.isValid () && stippleColor.isTransparent ()),
        &pack);
}

// public static
void kpPixmapFX::drawLine (QPixmap *image,
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


//
// drawPolygon()
//

struct DrawPolygonPackage
{
    QPolygon points;
    kpColor fcolor;
    int penWidth;
    kpColor bcolor;
    bool isFinal;
    kpColor fStippleColor;
};

static void DrawPolygonHelper (QPainter *p,
        bool drawingOnRGBLayer,
        void *data)
{
    DrawPolygonPackage *pack = static_cast <DrawPolygonPackage *> (data);

#if DEBUG_KP_PIXMAP_FX
    kDebug () << "DrawPolygonHelper() points=" << pack->points.toList ()
        << " fcolor=" << (int *) pack->fcolor.toQRgb ()
        << " penWidth=" << pack->penWidth
        << " bcolor="
        << (int *) (pack->bcolor.isValid () ?
                       pack->bcolor.toQRgb () :
                       0xabadcafe)
        << " isFinal=" << pack->isFinal
        << endl;
#endif

    ::QPainterSetPenWithStipple (p, drawingOnRGBLayer,
        pack->fcolor, pack->penWidth,
        pack->fStippleColor);

    if (pack->bcolor.isValid ())
        p->setBrush (QBrush (kpPixmapFX::draw_ToQColor (pack->bcolor, drawingOnRGBLayer)));
    // HACK: seems to be needed if set_Pen_(Qt::color0) else fills with Qt::color0.
    else
        p->setBrush (Qt::NoBrush);

    // Qt bug: single point doesn't show up depending on penWidth.
    if (Only1PixelInPointArray (pack->points))
    {
    #if DEBUG_KP_PIXMAP_FX
        kDebug () << "\tinvoking single point hack";
    #endif
        p->drawPoint (pack->points [0]);
        return;
    }

    // TODO: why aren't the ends rounded?
    p->drawPolygon (pack->points, Qt::OddEvenFill);

// TODO
#if 0
    if (pack->isFinal)
        return;

    int count = pointsInRect.count ();
    if (count <= 2)
        return;

    if (painter.isActive ())
    {
        QPen XORpen = painter.pen ();
        XORpen.setColor (Qt::white);

        painter.setPen (XORpen);
        painter.setRasterOp (Qt::XorROP);
    }

    if (maskPainter.isActive ())
    {
        QPen XORpen = maskPainter.pen ();

        // TODO???
        #if 0
        if (kpTool::isColorTransparent (foregroundColor))
            XORpen.setColor (Qt::color1/*opaque*/);
        else
            XORpen.setColor (Qt::color0/*transparent*/);
        #endif

        maskPainter.setPen (XORpen);
    }

    PAINTER_CALL (drawLine (pointsInRect [0], pointsInRect [count - 1]));
#endif
}

// public static
void kpPixmapFX::drawPolygon (QPixmap *image,
        const QPolygon &points,
        const kpColor &fcolor, int penWidth,
        const kpColor &bcolor,
        bool isFinal,
        const kpColor &fStippleColor)
{
    DrawPolygonPackage pack;
    pack.points = points;
    pack.fcolor = fcolor;
    pack.penWidth = penWidth;
    pack.bcolor = bcolor;
    pack.isFinal = isFinal;
    pack.fStippleColor = fStippleColor,

    kpPixmapFX::draw (image, &::DrawPolygonHelper,
        fcolor.isOpaque () ||
            (bcolor.isValid () && bcolor.isOpaque ()) ||
            (fStippleColor.isValid () && fStippleColor.isOpaque ()),
        fcolor.isTransparent () ||
            (bcolor.isValid () && bcolor.isTransparent ()) ||
            (fStippleColor.isValid () && fStippleColor.isTransparent ()),
        &pack);
}


//
// drawCurve()
//

struct DrawCurvePackage
{
    QPoint startPoint,
        controlPointP, controlPointQ,
        endPoint;
    kpColor color;
    int penWidth;
};

static void DrawCurveHelper (QPainter *p,
        bool drawingOnRGBLayer,
        void *data)
{
    DrawCurvePackage *pack = static_cast <DrawCurvePackage *> (data);

#if DEBUG_KP_PIXMAP_FX
    kDebug () << "DrawCurveHelper() pack: startPoint=" << pack->startPoint
        << " controlPointP=" << pack->controlPointP
        << " controlPointQ=" << pack->controlPointQ
        << " endPoint=" << pack->endPoint
        << " color=" << (int *) pack->color.toQRgb ()
        << " penWidth=" << pack->penWidth
        << endl;
#endif

    ::QPainterSetPenWithStipple (p, drawingOnRGBLayer,
        pack->color, pack->penWidth);

    // SYNC: Qt bug: single point doesn't show up depending on penWidth.
    if (pack->startPoint == pack->controlPointP &&
        pack->controlPointP == pack->controlPointQ &&
        pack->controlPointQ == pack->endPoint)
    {
    #if DEBUG_KP_PIXMAP_FX
        kDebug () << "\tinvoking single point hack";
    #endif
        p->drawPoint (pack->startPoint);
        return;
    }

    QPainterPath curvePath;
    curvePath.moveTo (pack->startPoint);
    curvePath.cubicTo (pack->controlPointP,
        pack->controlPointQ,
        pack->endPoint);

    p->strokePath (curvePath, p->pen ());
}

// public static
void kpPixmapFX::drawCurve (QPixmap *image,
    const QPoint &startPoint,
    const QPoint &controlPointP, const QPoint &controlPointQ,
    const QPoint &endPoint,
    const kpColor &color, int penWidth)
{
    DrawCurvePackage pack;
    pack.startPoint = startPoint;
    pack.controlPointP = controlPointP;
    pack.controlPointQ = controlPointQ;
    pack.endPoint = endPoint;
    pack.color = color;
    pack.penWidth = penWidth;

    kpPixmapFX::draw (image, &::DrawCurveHelper,
        color.isOpaque (), color.isTransparent (),
        &pack);
}


//
// fillRect()
//

struct FillRectPackage
{
    int x, y, width, height;
    kpColor color;
    kpColor stippleColor;
};

static void FillRectHelper (QPainter *p,
        bool drawingOnRGBLayer,
        void *data)
{
    FillRectPackage *pack = static_cast <FillRectPackage *> (data);

    if (!pack->stippleColor.isValid ())
    {
        p->fillRect (pack->x, pack->y, pack->width, pack->height,
            kpPixmapFX::draw_ToQColor (pack->color, drawingOnRGBLayer));
    }
    else
    {
        const int StippleSize = 4;

        p->save ();
        p->setClipRect (pack->x, pack->y, pack->width, pack->height);
        {
            for (int dy = 0; dy < pack->height; dy += StippleSize)
            {
                for (int dx = 0; dx < pack->width; dx += StippleSize)
                {
                    const bool parity = ((dy + dx) / StippleSize) % 2;
    
                    kpColor useColor;
                    if (!parity)
                        useColor = pack->color;
                    else
                        useColor = pack->stippleColor;
                        
                    p->fillRect (pack->x + dx, pack->y + dy, StippleSize, StippleSize,
                        kpPixmapFX::draw_ToQColor (useColor, drawingOnRGBLayer));
                }
            }
        }
        p->restore ();
    }
}

// public static
void kpPixmapFX::fillRect (QPixmap *image,
        int x, int y, int width, int height,
        const kpColor &color,
        const kpColor &stippleColor)
{
    FillRectPackage pack;
    pack.x = x, pack.y = y, pack.width = width, pack.height = height;
    pack.color = color;
    pack.stippleColor = stippleColor;

    kpPixmapFX::draw (image, &::FillRectHelper,
        color.isOpaque () || (stippleColor.isValid () && stippleColor.isOpaque ()),
        color.isTransparent () || (stippleColor.isValid () && stippleColor.isTransparent ()),
        &pack);
}


//
// DrawGenericRect()
//

struct DrawGenericRectPackage
{
    int x, y, width, height;
    void (*func) (QPainter * /*p*/,
            int /*x*/, int /*y*/,
            int /*width*/, int/*height*/);
    kpColor fcolor;
    int penWidth;
    kpColor bcolor;
    kpColor fStippleColor;
    bool isEllipseLike;
};

static void DrawGenericRectHelper (QPainter *p,
        bool drawingOnRGBLayer,
        void *data)
{
    DrawGenericRectPackage *pack = static_cast <DrawGenericRectPackage *> (data);

#if DEBUG_KP_PIXMAP_FX
    kDebug () << "\tkppixmapfx.cpp:DrawGenericRectHelper(drawingOnRGBLayer="
              << drawingOnRGBLayer << ") pack: "
              << pack->x << "," << pack->y << ","
              << pack->width << "," << pack->height << ",func=" << pack->func << ")"
              << " pen.color=" << (int *) pack->fcolor.toQRgb ()
              << " penWidth=" << pack->penWidth
              << " bcolor="
              << (int *) (pack->bcolor.isValid () ?
                             pack->bcolor.toQRgb () :
                             0xabadcafe)
              << " isEllipseLike=" << pack->isEllipseLike
              << endl;
#endif

    ::QPainterSetPenWithStipple (p, drawingOnRGBLayer,
        pack->fcolor, pack->penWidth,
        pack->fStippleColor,
        pack->isEllipseLike);

    if (pack->bcolor.isValid ())
        p->setBrush (QBrush (kpPixmapFX::draw_ToQColor (pack->bcolor, drawingOnRGBLayer)));
    // HACK: seems to be needed if set_Pen_(Qt::color0) else fills with Qt::color0.
    else
        p->setBrush (Qt::NoBrush);

    // Fight Qt behaviour of painting width = fill width + pen width
    // and height = fill height + pen height.  Get rid of pen width.
    (*pack->func) (p,
        pack->x + pack->penWidth / 2,
        pack->y + pack->penWidth / 2,
        pack->width - pack->penWidth,
        pack->height - pack->penWidth);
}

// Calls to drawRect(), drawRoundedRect() and drawEllipse() are
// forwarded here.  <func> is the respective QPainter function and
// may or may not be called.
static void DrawGenericRect (QPixmap *image,
        int x, int y, int width, int height,
        void (*func) (QPainter * /*p*/, int /*x*/, int /*y*/,
                int /*width*/, int/*height*/),
        const kpColor &fcolor, int penWidth,
        const kpColor &bcolor,
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


    Q_ASSERT (width > 0);
    Q_ASSERT (height > 0);

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


    DrawGenericRectPackage pack;
    pack.x = x, pack.y = y, pack.width = width, pack.height = height;
    pack.func = func;
    pack.fcolor = fcolor;

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
        pack.penWidth = 1;
        pack.bcolor = fcolor;  // Outline colour.
    }
    else
    {
        pack.penWidth = penWidth;
        pack.bcolor = bcolor;
    }

    pack.fStippleColor = fStippleColor;

    pack.isEllipseLike = isEllipseLike;


    kpPixmapFX::draw (image, &::DrawGenericRectHelper,
        fcolor.isOpaque () ||
            (bcolor.isValid () && bcolor.isOpaque ()) ||
            (fStippleColor.isValid () && fStippleColor.isOpaque ()),
        fcolor.isTransparent () ||
            (bcolor.isValid () && bcolor.isTransparent ()) ||
            (fStippleColor.isValid () && fStippleColor.isTransparent ()),
        &pack);
}


static void DrawRectHelper (QPainter *p,
        int x, int y, int width, int height)
{
    p->drawRect (x, y, width, height);
}

// public static
void kpPixmapFX::drawRect (QPixmap *image,
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


static void DrawRoundedRectHelper (QPainter *p,
        int x, int y, int width, int height)
{
    // (has default arguments for the roundness i.e. different prototype
    //  to QPainter::draw{Rect,Ellipse}(), therefore need pointer to these
    //  helpers instead of directly to a QPainter member function)
    p->drawRoundRect (x, y, width, height);
}

// public static
void kpPixmapFX::drawRoundedRect (QPixmap *image,
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


static void DrawEllipseHelper (QPainter *p,
        int x, int y, int width, int height)
{
    p->drawEllipse (x, y, width, height);
}

// public static
void kpPixmapFX::drawEllipse (QPixmap *image,
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
