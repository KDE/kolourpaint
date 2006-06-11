
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


#define DEBUG_KP_PAINTER 1


#include <kppainter.h>

#include <QBitmap>
#include <QPainter>
#include <QPainterPath>
#include <QPolygon>

#include <kdebug.h>

#include <kpimage.h>
#include <kppixmapfx.h>



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


// Exercises the drawing pattern on QPixmap's - draws on, separately, the:
//
// 1. RGB layer (if there is an opaque colour involved in the drawing i.e.
//               <anyColorOpaque>)
// 2. Mask layer (if there is a transparency involved i.e.
//                <anyColorTransparent> or the <image> has a mask to start
//                with)
//
// Each time, it opens up a QPainter and calls <drawFunc> with:
//
// 1. A pointer to this QPainter
// 2. A QColor member function that converts from kpColor to QColor
//    (depending on whether we are currently editing the RGB or mask layer)
// 3. A pointer to the provided <data>
static void Draw (kpImage *image,
        void (*drawFunc) (QPainter * /*p*/,
            QColor (kpColor::* /*toQColor*/) () const,
            void * /*data*/),
        bool anyColorOpaque, bool anyColorTransparent,
        void *data)
{
    // Get mask.
    QBitmap mask = image->mask ();

    // Draw on RGB layer?
    if (anyColorOpaque)
    {
        // RGB draw is not allowed to touch mask.
        image->setMask (QBitmap ());

        QPainter p (image);
        (*drawFunc) (&p, &kpColor::toQColor, data);
    }

    // Draw on mask layer?
    if (anyColorTransparent ||
        !mask.isNull ())
    {
        if (mask.isNull ())
            mask = kpPixmapFX::getNonNullMask (*image);
        
        QPainter p (&mask);
        (*drawFunc) (&p, &kpColor::maskColor, data);
    }

    // Set new mask.
    image->setMask (mask);
}

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


//
// drawPolyline() / drawLine()
//

struct DrawPolylinePackage
{
    QPolygon points;
    kpColor color;
    int penWidth;
};

static void DrawPolylineHelper (QPainter *p,
        QColor (kpColor::*toQColor) () const,
        void *data)
{
    DrawPolylinePackage *pack = static_cast <DrawPolylinePackage *> (data);

#if DEBUG_KP_PAINTER
    kDebug () << "DrawPolylineHelper() points=" << pack->points.toList ()
        << " color=" << (pack->color.isValid () ? pack->color.toQRgb () : 0xabadcafe)
        << " penWidth=" << pack->penWidth
        << endl;
#endif

    p->setPen (
        kpPixmapFX::QPainterDrawLinePen (
            (pack->color.*toQColor) (),
            ::WidthToQPenWidth (pack->penWidth)));
            
    // Qt bug: single point doesn't show up depending on penWidth.
    if (Only1PixelInPointArray (pack->points))
    {
    #if DEBUG_KP_PAINTER
        kDebug () << "\tinvoking single point hack" << endl;
    #endif
        p->drawPoint (pack->points [0]);
        return;
    }
    
    p->drawPolyline (pack->points);
}

// public static
void kpPainter::drawPolyline (kpImage *image,
        const QPolygon &points,
        const kpColor &color, int penWidth)
{
    DrawPolylinePackage pack;
    pack.points = points;
    pack.color = color;
    pack.penWidth = penWidth;
    
    ::Draw (image, &::DrawPolylineHelper,
        color.isOpaque (), color.isTransparent (),
        &pack);
}

// public static
void kpPainter::drawLine (kpImage *image,
        int x1, int y1, int x2, int y2,
        const kpColor &color, int penWidth)
{
    QPolygon points;
    points.append (QPoint (x1, y1));
    points.append (QPoint (x2, y2));
    
    drawPolyline (image,
        points,
        color, penWidth);
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
};

static void DrawPolygonHelper (QPainter *p,
        QColor (kpColor::*toQColor) () const,
        void *data)
{
    DrawPolygonPackage *pack = static_cast <DrawPolygonPackage *> (data);

#if DEBUG_KP_PAINTER
    kDebug () << "DrawPolygonHelper() points=" << pack->points.toList ()
        << " fcolor=" << (pack->fcolor.isValid () ? pack->fcolor.toQRgb () : 0xabadcafe)
        << " penWidth=" << pack->penWidth
        << " bcolor=" << (pack->bcolor.isValid () ? pack->bcolor.toQRgb () : 0xabadcafe)
        << " isFinal=" << pack->isFinal
        << endl;
#endif

    p->setPen (
        kpPixmapFX::QPainterDrawLinePen (
            (pack->fcolor.*toQColor) (),
            ::WidthToQPenWidth (pack->penWidth)));

    if (pack->bcolor.isValid ())
       p->setBrush (QBrush ((pack->bcolor.*toQColor) ()));
       
    // Qt bug: single point doesn't show up depending on penWidth.
    if (Only1PixelInPointArray (pack->points))
    {
    #if DEBUG_KP_PAINTER
        kDebug () << "\tinvoking single point hack" << endl;
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
void kpPainter::drawPolygon (kpImage *image,
        const QPolygon &points,
        const kpColor &fcolor, int penWidth,
        const kpColor &bcolor,
        bool isFinal)
{
    DrawPolygonPackage pack;
    pack.points = points;
    pack.fcolor = fcolor;
    pack.penWidth = penWidth;
    pack.bcolor = bcolor;
    pack.isFinal = isFinal;
    
    ::Draw (image, &::DrawPolygonHelper,
        fcolor.isOpaque () || (bcolor.isValid () && bcolor.isOpaque ()),
        fcolor.isTransparent () || (bcolor.isValid () && bcolor.isTransparent ()),
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
        QColor (kpColor::*toQColor) () const,
        void *data)
{
    DrawCurvePackage *pack = static_cast <DrawCurvePackage *> (data);

#if DEBUG_KP_PAINTER
    kDebug () << "DrawCurveHelper() pack: startPoint=" << pack->startPoint
        << " controlPointP=" << pack->controlPointP
        << " controlPointQ=" << pack->controlPointQ
        << " endPoint=" << pack->endPoint
        << " color=" << (pack->color.isValid () ? pack->color.toQRgb () : 0xabadcafe)
        << " penWidth=" << pack->penWidth
        << endl;
#endif

    const QPen curvePen =
        kpPixmapFX::QPainterDrawLinePen (
            (pack->color.*toQColor) (),
            ::WidthToQPenWidth (pack->penWidth));

    // SYNC: Qt bug: single point doesn't show up depending on penWidth.
    if (pack->startPoint == pack->controlPointP &&
        pack->controlPointP == pack->controlPointQ &&
        pack->controlPointQ == pack->endPoint)
    {
    #if DEBUG_KP_PAINTER
        kDebug () << "\tinvoking single point hack" << endl;
    #endif
        p->setPen (curvePen);
        p->drawPoint (pack->startPoint);
        return;
    }

    QPainterPath curvePath;
    curvePath.moveTo (pack->startPoint);
    curvePath.cubicTo (pack->controlPointP,
        pack->controlPointQ,
        pack->endPoint);
        
    p->strokePath (curvePath, curvePen);
}

// public static
void kpPainter::drawCurve (kpImage *image,
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
    
    ::Draw (image, &::DrawCurveHelper,
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
};

static void FillRectHelper (QPainter *p,
        QColor (kpColor::*toQColor) () const,
        void *data)
{
    FillRectPackage *pack = static_cast <FillRectPackage *> (data);

    p->fillRect (pack->x, pack->y, pack->width, pack->height,
        (pack->color.*toQColor) ());
}

// public static
void kpPainter::fillRect (kpImage *image,
        int x, int y, int width, int height,
        const kpColor &color)
{
    FillRectPackage pack;
    pack.x = x, pack.y = y, pack.width = width, pack.height = height;
    pack.color = color;

    ::Draw (image, &::FillRectHelper,
        color.isOpaque (), color.isTransparent (),
        &pack);
}


//
// RrawGenericRect()
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
    bool isEllipseLike;
};

static void DrawGenericRectHelper (QPainter *p,
        QColor (kpColor::*toQColor) () const,
        void *data)
{
    DrawGenericRectPackage *pack = static_cast <DrawGenericRectPackage *> (data);

#if DEBUG_KP_PAINTER
    kDebug () << "\tdraw adjusting for outline width" << endl;
#endif

    p->setPen (
        kpPixmapFX::QPainterDrawRectPen (
            (pack->fcolor.*toQColor) (),
            ::WidthToQPenWidth (pack->penWidth, pack->isEllipseLike)));

    if (pack->bcolor.isValid ())
        p->setBrush (QBrush ((pack->bcolor.*toQColor) ()));
    
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
static void DrawGenericRect (kpImage *image,
        int x, int y, int width, int height,
        void (*func) (QPainter * /*p*/, int /*x*/, int /*y*/,
                int /*width*/, int/*height*/),
        const kpColor &fcolor, int penWidth,
        const kpColor &bcolor,
        bool isEllipseLike = false)
{
#if DEBUG_KP_PAINTER
    kDebug () << "kpPainter::drawGenericRect(" << x << "," << y << ","
        << width << "," << height << ",func=" << func << ")" << endl;
#endif
        
    Q_ASSERT (width > 0);
    Q_ASSERT (height > 0);

    Q_ASSERT (func);

    // Check foreground colour valid.
    // Background is allowed to be invalid (no fill).
    Q_ASSERT (fcolor.isValid ());


    if (width == 1 || height == 1)
    {
    #if DEBUG_KP_PAINTER
        kDebug () << "\twidth=1 or height=1 - draw line" << endl;
    #endif

        kpPainter::drawLine (image,
            x, y, x + width - 1, y + height - 1,
            fcolor, 1/*force pen width to 1*/);
        return;
    }


    DrawGenericRectPackage pack;
    pack.x = x, pack.y = y, pack.width = width, pack.height = height;
    pack.func = func;
    pack.fcolor = fcolor;
    
    // Outline is so big that fill won't be seen?
    if (penWidth * 2 >= width || penWidth * 2 >= height)
    {
    #if DEBUG_KP_PAINTER
        kDebug () << "\toutline dominates fill - fill with outline" << endl;
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
    
    pack.isEllipseLike = isEllipseLike;


    ::Draw (image, &::DrawGenericRectHelper,
        fcolor.isOpaque () || (bcolor.isValid () && bcolor.isOpaque ()),
        fcolor.isTransparent () || (bcolor.isValid () && bcolor.isTransparent ()),
        &pack);
}


static void DrawRectHelper (QPainter *p,
        int x, int y, int width, int height)
{
    p->drawRect (x, y, width, height);
}

void kpPainter::drawRect (kpImage *image,
        int x, int y, int width, int height,
        const kpColor &fcolor, int penWidth,
        const kpColor &bcolor)
{
    ::DrawGenericRect (image,
        x, y, width, height,
        &::DrawRectHelper,
        fcolor, penWidth,
        bcolor);
}


static void DrawRoundedRectHelper (QPainter *p,
        int x, int y, int width, int height)
{
    // (has default arguments for the roundness i.e. different prototype
    //  to QPainter::draw{Rect,Ellipse}(), therefore need pointer to these
    //  helpers instead of directly to a QPainter member function)
    p->drawRoundRect (x, y, width, height);
}

void kpPainter::drawRoundedRect (kpImage *image,
        int x, int y, int width, int height,
        const kpColor &fcolor, int penWidth,
        const kpColor &bcolor)
{
    ::DrawGenericRect (image,
        x, y, width, height,
        &::DrawRoundedRectHelper,
        fcolor, penWidth,
        bcolor,
        true/*ellipse like*/);
}


static void DrawEllipseHelper (QPainter *p,
        int x, int y, int width, int height)
{
    p->drawEllipse (x, y, width, height);
}

void kpPainter::drawEllipse (kpImage *image,
        int x, int y, int width, int height,
        const kpColor &fcolor, int penWidth,
        const kpColor &bcolor)
{
    ::DrawGenericRect (image,
        x, y, width, height,
        &::DrawEllipseHelper,
        fcolor, penWidth,
        bcolor,
        true/*ellipse like*/);
}
