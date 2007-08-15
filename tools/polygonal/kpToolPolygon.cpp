
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


#define DEBUG_KP_TOOL_POLYGON 0


#include <kpToolPolygon.h>

#include <klocale.h>

#include <kpPainter.h>
#include <kpToolToolBar.h>


static void DrawPolygonShape (kpImage *image,
        const QPolygon &points,
        const kpColor &fcolor, int penWidth,
        const kpColor &bcolor,
        bool isFinal)
{
    kpPainter::drawPolygon (image,
        points,
        fcolor, penWidth,
        bcolor,
        isFinal);
}


struct kpToolPolygonPrivate
{
    kpToolWidgetFillStyle *toolWidgetFillStyle;
};

kpToolPolygon::kpToolPolygon (kpToolEnvironment *environ, QObject *parent)
    : kpToolPolygonalBase (
        i18n ("Polygon"),
        i18n ("Draws polygons"),
        &::DrawPolygonShape,
        Qt::Key_G,
        environ, parent,
        "tool_polygon"),
      d (new kpToolPolygonPrivate ())
{
}

kpToolPolygon::~kpToolPolygon ()
{
    delete d;
}


// private virtual [base kpToolPolygonBase]
QString kpToolPolygon::haventBegunShapeUserMessage () const
{
    return i18n ("Drag to draw the first line.");
}


// public virtual [base kpToolPolygonalBase]
void kpToolPolygon::begin ()
{
    kpToolPolygonalBase::begin ();

    kpToolToolBar *tb = toolToolBar ();
    Q_ASSERT (tb);

    d->toolWidgetFillStyle = tb->toolWidgetFillStyle ();
    connect (d->toolWidgetFillStyle,
        SIGNAL (fillStyleChanged (kpToolWidgetFillStyle::FillStyle)),
        this,
        SLOT (updateShape ()));
    d->toolWidgetFillStyle->show ();
}

// public virtual [base kpToolPolygonalBase]
void kpToolPolygon::end ()
{
    kpToolPolygonalBase::end ();

    disconnect (d->toolWidgetFillStyle,
        SIGNAL (fillStyleChanged (kpToolWidgetFillStyle::FillStyle)),
        this,
        SLOT (updateShape ()));
    d->toolWidgetFillStyle = 0;
}


// TODO: code dup with kpToolRectangle
// protected virtual [base kpToolPolygonalBase]
kpColor kpToolPolygon::drawingBackgroundColor () const
{
    const kpColor foregroundColor = color (originatingMouseButton ());
    const kpColor backgroundColor = color (1 - originatingMouseButton ());

    return d->toolWidgetFillStyle->drawingBackgroundColor (
        foregroundColor, backgroundColor);
}


// public virtual [base kpTool]
// TODO: dup with kpToolPolyline but we don't want to create another level of
//       inheritance and readability.
void kpToolPolygon::endDraw (const QPoint &, const QRect &)
{
#if DEBUG_KP_TOOL_POLYGON
    kDebug () << "kpToolPolygon::endDraw()  points="
        << points ()->toList () << endl;
#endif

    // A click of the other mouse button (to finish shape, instead of adding
    // another control point) would have caused endShape() to have been
    // called in kpToolPolygonalBase::beginDraw().  The points list would now
    // be empty.  We are being called by kpTool::mouseReleaseEvent().
    if (points ()->count () == 0)
        return;

    if (points ()->count () >= kpToolPolygonalBase::MaxPoints)
    {
    #if DEBUG_KP_TOOL_POLYGON
        kDebug () << "\tending shape";
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


#include <kpToolPolygon.moc>
