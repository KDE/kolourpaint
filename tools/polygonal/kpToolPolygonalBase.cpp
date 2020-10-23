
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


#include "kpToolPolygonalBase.h"

#include <cfloat>
#include <QtMath>

#include <QCursor>
#include <QPolygon>

#include <KLocalizedString>

#include "kpLogCategories.h"
#include "commands/kpCommandHistory.h"
#include "document/kpDocument.h"
#include "kpDefs.h"
#include "imagelib/kpPainter.h"
#include "pixmapfx/kpPixmapFX.h"
#include "layers/tempImage/kpTempImage.h"
#include "environments/tools/kpToolEnvironment.h"
#include "commands/tools/polygonal/kpToolPolygonalCommand.h"
#include "widgets/toolbars/kpToolToolBar.h"
#include "widgets/toolbars/options/kpToolWidgetLineWidth.h"
#include "views/manager/kpViewManager.h"


struct kpToolPolygonalBasePrivate
{
    kpToolPolygonalBasePrivate ()
        : drawShapeFunc(nullptr), toolWidgetLineWidth(nullptr), originatingMouseButton(-1)
    {
    }

    kpToolPolygonalBase::DrawShapeFunc drawShapeFunc;

    kpToolWidgetLineWidth *toolWidgetLineWidth;

    int originatingMouseButton;

    QPolygon points;
};

//---------------------------------------------------------------------

kpToolPolygonalBase::kpToolPolygonalBase (
        const QString &text,
        const QString &description,
        DrawShapeFunc drawShapeFunc,
        int key,
        kpToolEnvironment *environ, QObject *parent,
        const QString &name)

    : kpTool (text, description, key, environ, parent, name),
      d (new kpToolPolygonalBasePrivate ())
{
    d->drawShapeFunc = drawShapeFunc;

    d->toolWidgetLineWidth = nullptr;

    // (hopefully cause crash if we use it before initialising it)
    d->originatingMouseButton = -1;
}

//---------------------------------------------------------------------

kpToolPolygonalBase::~kpToolPolygonalBase ()
{
    delete d;
}

//---------------------------------------------------------------------

// virtual
void kpToolPolygonalBase::begin ()
{
    kpToolToolBar *tb = toolToolBar ();
    Q_ASSERT (tb);

#if DEBUG_KP_TOOL_POLYGON
    qCDebug(kpLogTools) << "kpToolPolygonalBase::begin() tb=" << tb;
#endif

    d->toolWidgetLineWidth = tb->toolWidgetLineWidth ();
    connect (d->toolWidgetLineWidth, &kpToolWidgetLineWidth::lineWidthChanged,
             this, &kpToolPolygonalBase::updateShape);
    d->toolWidgetLineWidth->show ();

    viewManager ()->setCursor (QCursor (Qt::ArrowCursor));

    d->originatingMouseButton = -1;

    setUserMessage (/*virtual*/haventBegunShapeUserMessage ());
}

//---------------------------------------------------------------------

// virtual
void kpToolPolygonalBase::end ()
{
    // TODO: needed?
    endShape ();

    disconnect (d->toolWidgetLineWidth, &kpToolWidgetLineWidth::lineWidthChanged,
             this, &kpToolPolygonalBase::updateShape);
    d->toolWidgetLineWidth = nullptr;

    viewManager ()->unsetCursor ();
}


void kpToolPolygonalBase::beginDraw ()
{
#if DEBUG_KP_TOOL_POLYGON
    qCDebug(kpLogTools) << "kpToolPolygonalBase::beginDraw()  d->points=" << d->points.toList ()
               << ", startPoint=" << startPoint ();
#endif

    bool endedShape = false;

    // We now need to start with dragging out the initial line?
    if (d->points.count () == 0)
    {
        d->originatingMouseButton = mouseButton ();

        // The line starts and ends at the start point of the drag.
        // draw() will modify the last point in d->points to reflect the
        // mouse drag, as the drag proceeds.
        d->points.append (startPoint ());
        d->points.append (startPoint ());
    }
    // Already have control points - not dragging out initial line.
    else
    {
        // Clicking the other mouse button?
        if (mouseButton () != d->originatingMouseButton)
        {
            // Finish shape.  TODO: I suspect we need to call endShapeInternal instead.
            endShape ();
            endedShape = true;
        }
        // Are we dragging out an extra control point?
        else
        {
            // Add another control point.
            d->points.append (startPoint ());
        }
    }

#if DEBUG_KP_TOOL_POLYGON
    qCDebug(kpLogTools) << "\tafterwards, d->points=" << d->points.toList ();
#endif

    if (!endedShape)
    {
        // We've started dragging.  Print instructions on how to cancel shape.
        setUserMessage (cancelUserMessage ());
    }
}


// protected
void kpToolPolygonalBase::applyModifiers ()
{
    const int count = d->points.count ();

    QPoint &lineStartPoint = d->points [count - 2];
    QPoint &lineEndPoint = d->points [count - 1];

#if DEBUG_KP_TOOL_POLYGON && 1
    qCDebug(kpLogTools) << "kpToolPolygonalBase::applyModifiers() #pts=" << count
               << "   line: startPt=" << lineStartPoint
               << " endPt=" << lineEndPoint
               << "   modifiers: shift=" << shiftPressed ()
               << "   alt=" << altPressed ()
               << "   ctrl=" << controlPressed ();
#endif

    // angles
    if (shiftPressed () || controlPressed ())
    {
        int diffx = lineEndPoint.x () - lineStartPoint.x ();
        int diffy = lineEndPoint.y () - lineStartPoint.y ();

        double ratio;
        if (diffx == 0) {
            ratio = DBL_MAX;
        }
        else {
            ratio = fabs (double (diffy) / double (diffx));
        }
    #if DEBUG_KP_TOOL_POLYGON && 1
        qCDebug(kpLogTools) << "\tdiffx=" << diffx << " diffy=" << diffy
                   << " ratio=" << ratio;
    #endif

        // Shift        = 0, 45, 90
        // Ctrl         = 0, 30, 60, 90
        // Shift + Ctrl = 0, 30, 45, 60, 90
        double angles [10];  // "ought to be enough for anybody"
        int numAngles = 0;
        angles [numAngles++] = 0;
        if (controlPressed ()) {
            angles [numAngles++] = M_PI / 6;
        }
        if (shiftPressed ()) {
            angles [numAngles++] = M_PI / 4;
        }
        if (controlPressed ()) {
            angles [numAngles++] = M_PI / 3;
        }
        angles [numAngles++] = M_PI / 2;
        Q_ASSERT (numAngles <= int (sizeof (angles) / sizeof (angles [0])));

        double angle = angles [numAngles - 1];
        for (int i = 0; i < numAngles - 1; i++)
        {
            double acceptingRatio = std::tan ((angles [i] + angles [i + 1]) / 2.0);
            if (ratio < acceptingRatio)
            {
                angle = angles [i];
                break;
            }
        }

        // horizontal (dist from start not maintained)
        if (std::fabs (qRadiansToDegrees (angle) - 0)
            < kpPixmapFX::AngleInDegreesEpsilon)
        {
            lineEndPoint =
                QPoint (lineEndPoint.x (), lineStartPoint.y ());
        }
        // vertical (dist from start not maintained)
        else if (std::fabs (qRadiansToDegrees (angle) - 90)
                 < kpPixmapFX::AngleInDegreesEpsilon)
        {
            lineEndPoint =
                QPoint (lineStartPoint.x (), lineEndPoint.y ());
        }
        // diagonal (dist from start maintained)
        else
        {
            const double dist = std::sqrt (static_cast<double> (diffx * diffx + diffy * diffy));

            #define sgn(a) ((a)<0?-1:1)
            // Round distances _before_ adding to any coordinate
            // (ensures consistent rounding behaviour in x & y directions)
            const int newdx = qRound (dist * cos (angle) * sgn (diffx));
            const int newdy = qRound (dist * sin (angle) * sgn (diffy));
            #undef sgn

            lineEndPoint = QPoint (lineStartPoint.x () + newdx,
                                         lineStartPoint.y () + newdy);

        #if DEBUG_KP_TOOL_POLYGON && 1
            qCDebug(kpLogTools) << "\t\tdiagonal line: dist=" << dist
                       << " angle=" << (angle * 180 / M_PI)
                       << " endPoint=" << lineEndPoint;
        #endif
        }
    }    // if (shiftPressed () || controlPressed ()) {

    // centring
    if (altPressed () && 0/*ALT is unreliable*/)
    {
        // start = start - diff
        //       = start - (end - start)
        //       = start - end + start
        //       = 2 * start - end
        if (count == 2) {
            lineStartPoint += (lineStartPoint - lineEndPoint);
        }
        else {
            lineEndPoint += (lineEndPoint - lineStartPoint);
        }
    }    // if (altPressed ()) {
}


// protected
QPolygon *kpToolPolygonalBase::points () const
{
    return &d->points;
}


// protected
int kpToolPolygonalBase::originatingMouseButton () const
{
    Q_ASSERT (hasBegunShape ());
    return d->originatingMouseButton;
}


// virtual
void kpToolPolygonalBase::draw (const QPoint &, const QPoint &, const QRect &)
{
    // A click of the other mouse button (to finish shape, instead of adding
    // another control point) would have caused endShape() to have been
    // called in kpToolPolygonalBase::beginDraw().  The points list would now
    // be empty.  We are being called by kpTool::mouseReleaseEvent().
    if (d->points.count () == 0) {
        return;
    }

#if DEBUG_KP_TOOL_POLYGON
    qCDebug(kpLogTools) << "kpToolPolygonalBase::draw()  d->points=" << d->points.toList ()
               << ", endPoint=" << currentPoint ();
#endif

    // Update points() so that last point reflects current mouse position.
    const int count = d->points.count ();
    d->points [count - 1] = currentPoint ();

#if DEBUG_KP_TOOL_POLYGON
    qCDebug(kpLogTools) << "\tafterwards, d->points=" << d->points.toList ();
#endif

    // Are we drawing a line?
    if (/*virtual*/drawingALine ())
    {
        // Adjust the line (end points given by the last 2 points of points())
        // in response to keyboard modifiers.
        applyModifiers ();

        // Update the preview of the shape.
        updateShape ();

        // Inform the user that we're dragging out a line with 2 control points.
        setUserShapePoints (d->points [count - 2], d->points [count - 1]);
    }
    // We're modifying a point.
    else
    {
        // Update the preview of the shape.
        updateShape ();

        // Informs the user that we're just modifying a point (perhaps, a control
        // point of a Bezier).
        setUserShapePoints (d->points [count - 1]);
    }
}


// TODO: code dup with kpToolRectangle
// private
kpColor kpToolPolygonalBase::drawingForegroundColor () const
{
    return color (originatingMouseButton ());
}

// protected virtual
kpColor kpToolPolygonalBase::drawingBackgroundColor () const
{
    return kpColor::Invalid;
}

// TODO: code dup with kpToolRectangle
// protected slot
void kpToolPolygonalBase::updateShape ()
{
    if (d->points.count () == 0) {
        return;
    }

    const QRect boundingRect = kpTool::neededRect (
            d->points.boundingRect (),
            d->toolWidgetLineWidth->lineWidth ());

#if DEBUG_KP_TOOL_POLYGON
    qCDebug(kpLogTools) << "kpToolPolygonalBase::updateShape() boundingRect="
               << boundingRect
               << " lineWidth="
               << d->toolWidgetLineWidth->lineWidth ()
               << endl;
#endif

    kpImage image = document ()->getImageAt (boundingRect);

    QPolygon pointsTranslated = d->points;
    pointsTranslated.translate (-boundingRect.x (), -boundingRect.y ());

    (*d->drawShapeFunc) (&image,
        pointsTranslated,
        drawingForegroundColor (), d->toolWidgetLineWidth->lineWidth (),
        /*virtual*/drawingBackgroundColor (),
        false/*not final*/);

    kpTempImage newTempImage (false/*always display*/,
                                kpTempImage::SetImage/*render mode*/,
                                boundingRect.topLeft (),
                                image);

    viewManager ()->setFastUpdates ();
    {
        viewManager ()->setTempImage (newTempImage);
    }
    viewManager ()->restoreFastUpdates ();
}

// virtual
void kpToolPolygonalBase::cancelShape ()
{
    viewManager ()->invalidateTempImage ();
    d->points.resize (0);

    setUserMessage (i18n ("Let go of all the mouse buttons."));
}

void kpToolPolygonalBase::releasedAllButtons ()
{
    if (!hasBegunShape ()) {
        setUserMessage (/*virtual*/haventBegunShapeUserMessage ());
    }

    // --- else case already handled by endDraw() ---
}

// public virtual [base kpTool]
void kpToolPolygonalBase::endShape (const QPoint &, const QRect &)
{
#if DEBUG_KP_TOOL_POLYGON
    qCDebug(kpLogTools) << "kpToolPolygonalBase::endShape()  d->points="
        << d->points.toList () << endl;
#endif

    if (!hasBegunShape ()) {
        return;
    }

    viewManager ()->invalidateTempImage ();

    QRect boundingRect = kpTool::neededRect (
        d->points.boundingRect (),
        d->toolWidgetLineWidth->lineWidth ());

    commandHistory ()->addCommand (
        new kpToolPolygonalCommand (
            text (),
            d->drawShapeFunc,
            d->points, boundingRect,
            drawingForegroundColor (), d->toolWidgetLineWidth->lineWidth (),
            /*virtual*/drawingBackgroundColor (),
            environ ()->commandEnvironment ()));

    d->points.resize (0);
    setUserMessage (/*virtual*/haventBegunShapeUserMessage ());

}

// public virtual [base kpTool]
bool kpToolPolygonalBase::hasBegunShape () const
{
    return (d->points.count () > 0);
}


// virtual protected slot [base kpTool]
void kpToolPolygonalBase::slotForegroundColorChanged (const kpColor &)
{
    updateShape ();
}

// virtual protected slot [base kpTool]
void kpToolPolygonalBase::slotBackgroundColorChanged (const kpColor &)
{
    updateShape ();
}


