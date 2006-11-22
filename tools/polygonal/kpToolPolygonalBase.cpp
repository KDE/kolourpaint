
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


#define DEBUG_KP_TOOL_POLYGON 0


#include <kpToolPolygonalBase.h>

#include <float.h>
#include <math.h>

#include <qbitmap.h>
#include <qcursor.h>
#include <qlayout.h>
#include <qpainter.h>
#include <qpixmap.h>
#include <qpoint.h>
#include <qpolygon.h>
#include <qpushbutton.h>
#include <qrect.h>
#include <qtooltip.h>

#include <kdebug.h>
#include <klocale.h>

#include <kpbug.h>
#include <kpcommandhistory.h>
#include <kpdocument.h>
#include <kpdefs.h>
#include <kpimage.h>
#include <kpmainwindow.h>
#include <kppainter.h>
#include <kppixmapfx.h>
#include <kptemppixmap.h>
#include <kpToolPolygonalCommand.h>
#include <kptooltoolbar.h>
#include <kptoolwidgetlinewidth.h>
#include <kpviewmanager.h>


kpImage kpToolPolygonalBaseImage (const QPixmap &oldImage,
        const QPolygon &points, const QRect &rect,
        const kpColor &foregroundColor, int penWidth,
        kpColor backgroundColor,
        enum kpToolPolygonalBase::Mode mode, bool final)
{
#if DEBUG_KP_TOOL_POLYGON
    kDebug () << "kptoolpolygonalbase.cpp: image(): points=" << points.toList () << endl;
#endif

    kpImage image = oldImage;
    
    // Figure out points to draw relative to topLeft of oldPixmap.
    QPolygon pointsInRect = points;
    pointsInRect.detach ();
    pointsInRect.translate (-rect.x (), -rect.y ());

    switch (mode)
    {
    case kpToolPolygonalBase::Line:
    case kpToolPolygonalBase::Polyline:
        kpPainter::drawPolyline (&image,
            pointsInRect,
            foregroundColor, penWidth);
        break;

    case kpToolPolygonalBase::Polygon:
        kpPainter::drawPolygon (&image,
            pointsInRect,
            foregroundColor, penWidth,
            backgroundColor,
            final);
        break;

    case kpToolPolygonalBase::Curve:
    {
        const QPoint startPoint = pointsInRect [0];
        const QPoint endPoint = pointsInRect [1];

        QPoint controlPointP, controlPointQ;
        
        switch (pointsInRect.count ())
        {
        // Just a line?
        case 2:
            controlPointP = startPoint;
            controlPointQ = endPoint;
            break;

        // Single control point?
        case 3:
            controlPointP = controlPointQ = pointsInRect [2];
            break;

        // Two control points?
        case 4:
            controlPointP = pointsInRect [2];
            controlPointQ = pointsInRect [3];
            break;

        default:
            kError () << "kptoolpolygonalbase.cpp:image() pointsInRect.count="
                << pointsInRect.count () << endl;
            break;
        }

        kpPainter::drawCurve (&image,
            startPoint,
            controlPointP, controlPointQ,
            endPoint,
            foregroundColor, penWidth);
    }}

    return image;
}


//
// kpToolPolygonalBase
//

struct kpToolPolygonalBasePrivate
{
    kpToolPolygonalBasePrivate ()
        : toolWidgetLineWidth (0)
    {
    }
    
    kpToolPolygonalBase::Mode mode;

    kpToolWidgetLineWidth *toolWidgetLineWidth;

    int originatingMouseButton;

    QPoint toolLineStartPoint, toolLineEndPoint;

    QPolygon points;
};

kpToolPolygonalBase::kpToolPolygonalBase (Mode mode,
                              const QString &text, const QString &description,
                              int key,
                              kpMainWindow *mainWindow, const QString &name)
    : kpTool (text, description, key, mainWindow, name),
      d (new kpToolPolygonalBasePrivate ())
{
    d->mode = mode;
}

kpToolPolygonalBase::kpToolPolygonalBase (kpMainWindow *mainWindow)
    : kpTool (i18n ("Polygon"), i18n ("Draws polygons"),
              Qt::Key_G,
              mainWindow, "tool_polygon"),
      d (new kpToolPolygonalBasePrivate ())
{
    d->mode = Polygon;
}

kpToolPolygonalBase::~kpToolPolygonalBase ()
{
    delete d;
}

void kpToolPolygonalBase::setMode (Mode m)
{
    d->mode = m;
}


// virtual
void kpToolPolygonalBase::begin ()
{
    kpToolToolBar *tb = toolToolBar ();
    Q_ASSERT (tb);

#if DEBUG_KP_TOOL_POLYGON
    kDebug () << "kpToolPolygonalBase::begin() tb=" << tb << endl;
#endif

    d->toolWidgetLineWidth = tb->toolWidgetLineWidth ();
    connect (d->toolWidgetLineWidth, SIGNAL (lineWidthChanged (int)),
                this, SLOT (updateShape ()));
    d->toolWidgetLineWidth->show ();

    viewManager ()->setCursor (QCursor (Qt::CrossCursor));

    d->originatingMouseButton = -1;

    setUserMessage (/*virtual*/haventBegunShapeUserMessage ());
}

// virtual
void kpToolPolygonalBase::end ()
{
    // TODO: needed?
    endShape ();

    disconnect (d->toolWidgetLineWidth,
        SIGNAL (lineWidthChanged (int)),
        this,
        SLOT (updateShape ()));
    d->toolWidgetLineWidth = 0;

    viewManager ()->unsetCursor ();
}


void kpToolPolygonalBase::beginDraw ()
{
#if DEBUG_KP_TOOL_POLYGON
    kDebug () << "kpToolPolygonalBase::beginDraw()  d->points=" << d->points.toList ()
               << ", startPoint=" << startPoint () << endl;
#endif

    bool endedShape = false;

    // We now need to start with dragging out the initial line?
    if (d->points.count () == 0)
    {
        d->originatingMouseButton = mouseButton ();
        
        d->points.append (startPoint ());
        d->points.append (startPoint ());
    }
    // Already have control points - not dragging out initial line.
    else
    {
        // Clicking the other mouse button?
        if (mouseButton () != d->originatingMouseButton)
        {
            // HITODO: what for?
            // mouseButton () = d->originatingMouseButton;
            
            // Finish shape.  TODO: I suspect we need to call endShapeInternal instead.
            endShape ();
            endedShape = true;
        }
        // Are we dragging out an extra control point?
        else
        {
            int count = d->points.count ();

            // Add another control point.
            d->points.append (startPoint ());

            // start point = last end point;
            // _not_ the new/current start point
            // (which is disregarded in a poly* as only the end points count
            //  after the initial line)
            //
            // Curve Tool ignores startPoint () (doesn't call applyModifiers())
            // after the initial has been defined.
            startPoint () = d->points [count - 1];
        }
    }

#if DEBUG_KP_TOOL_POLYGON
    kDebug () << "\tafterwards, d->points=" << d->points.toList () << endl;
#endif

    if (!endedShape)
    {
        // We've started dragging.  Print instructions on how to cancel shape.
        setUserMessage (cancelUserMessage ());
    }
}

// private
void kpToolPolygonalBase::applyModifiers ()
{
    int count = d->points.count ();

    d->toolLineStartPoint = startPoint ();  /* also correct for poly* tool (see beginDraw()) */
    d->toolLineEndPoint = currentPoint ();

#if DEBUG_KP_TOOL_POLYGON && 1
    kDebug () << "kpToolPolygonalBase::applyModifiers() #pts=" << count
               << "   line: startPt=" << d->toolLineStartPoint
               << " endPt=" << d->toolLineEndPoint
               << "   modifiers: shift=" << shiftPressed ()
               << "   alt=" << altPressed ()
               << "   ctrl=" << controlPressed ()
               << endl;
#endif

    // angles
    if (shiftPressed () || controlPressed ())
    {
        int diffx = d->toolLineEndPoint.x () - d->toolLineStartPoint.x ();
        int diffy = d->toolLineEndPoint.y () - d->toolLineStartPoint.y ();

        double ratio;
        if (diffx == 0)
            ratio = DBL_MAX;
        else
            ratio = fabs (double (diffy) / double (diffx));
    #if DEBUG_KP_TOOL_POLYGON && 1
        kDebug () << "\tdiffx=" << diffx << " diffy=" << diffy
                   << " ratio=" << ratio
                   << endl;
    #endif

        // Shift        = 0, 45, 90
        // Alt          = 0, 30, 60, 90
        // Shift + Alt  = 0, 30, 45, 60, 90
        double angles [10];  // "ought to be enough for anybody"
        int numAngles = 0;
        angles [numAngles++] = 0;
        if (controlPressed ())
            angles [numAngles++] = KP_PI / 6;
        if (shiftPressed ())
            angles [numAngles++] = KP_PI / 4;
        if (controlPressed ())
            angles [numAngles++] = KP_PI / 3;
        angles [numAngles++] = KP_PI / 2;

        double angle = angles [numAngles - 1];
        for (int i = 0; i < numAngles - 1; i++)
        {
            double acceptingRatio = tan ((angles [i] + angles [i + 1]) / 2.0);
            if (ratio < acceptingRatio)
            {
                angle = angles [i];
                break;
            }
        }

        // horizontal (dist from start not maintained)
        if (fabs (KP_RADIANS_TO_DEGREES (angle) - 0)
            < kpPixmapFX::AngleInDegreesEpsilon)
        {
            d->toolLineEndPoint =
                QPoint (d->toolLineEndPoint.x (), d->toolLineStartPoint.y ());
        }
        // vertical (dist from start not maintained)
        else if (fabs (KP_RADIANS_TO_DEGREES (angle) - 90)
                 < kpPixmapFX::AngleInDegreesEpsilon)
        {
            d->toolLineEndPoint =
                QPoint (d->toolLineStartPoint.x (), d->toolLineEndPoint.y ());
        }
        // diagonal (dist from start maintained)
        else
        {
            const double dist = sqrt (diffx * diffx + diffy * diffy);

            #define sgn(a) ((a)<0?-1:1)
            // Round distances _before_ adding to any coordinate
            // (ensures consistent rounding behaviour in x & y directions)
            const int newdx = qRound (dist * cos (angle) * sgn (diffx));
            const int newdy = qRound (dist * sin (angle) * sgn (diffy));
            #undef sgn

            d->toolLineEndPoint = QPoint (d->toolLineStartPoint.x () + newdx,
                                         d->toolLineStartPoint.y () + newdy);

        #if DEBUG_KP_TOOL_POLYGON && 1
            kDebug () << "\t\tdiagonal line: dist=" << dist
                       << " angle=" << (angle * 180 / KP_PI)
                       << " endPoint=" << d->toolLineEndPoint
                       << endl;
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
        if (count == 2)
            d->toolLineStartPoint += (d->toolLineStartPoint - d->toolLineEndPoint);
        else
            d->toolLineEndPoint += (d->toolLineEndPoint - d->toolLineStartPoint);
    }    // if (altPressed ()) {

    d->points [count - 2] = d->toolLineStartPoint;
    d->points [count - 1] = d->toolLineEndPoint;
}

QPolygon *kpToolPolygonalBase::points () const
{
    return &d->points;
}

// virtual
void kpToolPolygonalBase::draw (const QPoint &, const QPoint &, const QRect &)
{
    if (d->points.count () == 0)
        return;

#if DEBUG_KP_TOOL_POLYGON
    kDebug () << "kpToolPolygonalBase::draw()  d->points=" << d->points.toList ()
               << ", endPoint=" << currentPoint () << endl;
#endif

    bool drawingALine = (d->mode != Curve) ||
                        (d->mode == Curve && d->points.count () == 2);

    if (drawingALine)
        applyModifiers ();
    else
        d->points [d->points.count () - 1] = currentPoint ();

#if DEBUG_KP_TOOL_POLYGON
    kDebug () << "\tafterwards, d->points=" << d->points.toList () << endl;
#endif

    updateShape ();

    if (drawingALine)
        setUserShapePoints (d->toolLineStartPoint, d->toolLineEndPoint);
    else
        setUserShapePoints (currentPoint ());
}


// TODO: code dup with kpToolRectangle
// private
kpColor kpToolPolygonalBase::drawingForegroundColor () const
{
    return color (mouseButton ());
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
    if (d->points.count () == 0)
        return;

    QRect boundingRect = kpTool::neededRect (d->points.boundingRect (),
        d->toolWidgetLineWidth->lineWidth ());

#if DEBUG_KP_TOOL_POLYGON
    kDebug () << "kpToolPolygonalBase::updateShape() boundingRect="
               << boundingRect
               << " lineWidth="
               << d->toolWidgetLineWidth->lineWidth ()
               << endl;
#endif

    QPixmap oldPixmap = document ()->getPixmapAt (boundingRect);
    QPixmap newPixmap = ::kpToolPolygonalBaseImage (oldPixmap,
        d->points, boundingRect,
        drawingForegroundColor (), d->toolWidgetLineWidth->lineWidth (),
        /*virtual*/drawingBackgroundColor (),
        d->mode, false/*not final*/);

    viewManager ()->setFastUpdates ();
    viewManager ()->setTempPixmap (kpTempPixmap (false/*always display*/,
                                                 kpTempPixmap::SetPixmap/*render mode*/,
                                                 boundingRect.topLeft (),
                                                 newPixmap));
    viewManager ()->restoreFastUpdates ();
}

// virtual
void kpToolPolygonalBase::cancelShape ()
{
    viewManager ()->invalidateTempPixmap ();
    d->points.resize (0);

    setUserMessage (i18n ("Let go of all the mouse buttons."));
}

void kpToolPolygonalBase::releasedAllButtons ()
{
    if (!hasBegunShape ())
        setUserMessage (/*virtual*/haventBegunShapeUserMessage ());

    // --- else case already handled by endDraw() ---
}

// public virtual [base kpTool]
void kpToolPolygonalBase::endShape (const QPoint &, const QRect &)
{
#if DEBUG_KP_TOOL_POLYGON
    kDebug () << "kpToolPolygonalBase::endShape()  d->points="
        << d->points.toList () << endl;
#endif

    if (!hasBegunShape ())
        return;

    viewManager ()->invalidateTempPixmap ();

    QRect boundingRect = kpTool::neededRect (d->points.boundingRect (), d->toolWidgetLineWidth->lineWidth ());

    kpToolPolygonalCommand *lineCommand =
        new kpToolPolygonalCommand (
            text (),
            d->points, boundingRect,
            drawingForegroundColor (), d->toolWidgetLineWidth->lineWidth (),
            /*virtual*/drawingBackgroundColor (),
            document ()->getPixmapAt (boundingRect),
            d->mode,
            mainWindow ());

    commandHistory ()->addCommand (lineCommand);

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


#include <kpToolPolygonalBase.moc>
