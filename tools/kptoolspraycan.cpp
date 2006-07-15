
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


#define DEBUG_KP_TOOL_SPRAYCAN 1


#include <cstdlib>

#include <qbitmap.h>
#include <qpainter.h>
#include <qpen.h>
#include <qpixmap.h>
#include <qpoint.h>
#include <qpolygon.h>
#include <qrect.h>
#include <qtimer.h>

#include <kdebug.h>
#include <klocale.h>

#include <kpbug.h>
#include <kpcommandhistory.h>
#include <kpdefs.h>
#include <kpdocument.h>
#include <kpmainwindow.h>
#include <kppixmapfx.h>
#include <kptoolspraycan.h>
#include <kptoolflowcommand.h>
#include <kptooltoolbar.h>
#include <kptoolwidgetspraycansize.h>
#include <kpview.h>
#include <kpviewmanager.h>


kpToolSpraycan::kpToolSpraycan (kpMainWindow *mainWindow)
    : kpToolFlowBase (i18n ("Spraycan"), i18n ("Sprays graffiti"),
        Qt::Key_Y,
        mainWindow, "tool_spraycan")
{
    m_timer = new QTimer (this);
    connect (m_timer, SIGNAL (timeout ()),
        this, SLOT (timeoutDraw ()));
}

kpToolSpraycan::~kpToolSpraycan ()
{
}


// protected virtual [base kpToolFlowBase]
QString kpToolSpraycan::haventBegunDrawUserMessage () const
{
    return i18n ("Click or drag to spray graffiti.");
}


// public virtual [base kpToolFlowBase]
void kpToolSpraycan::begin ()
{
    kpToolToolBar *tb = toolToolBar ();
    Q_ASSERT (tb);

    m_toolWidgetSpraycanSize = tb->toolWidgetSpraycanSize ();
    connect (m_toolWidgetSpraycanSize, SIGNAL (spraycanSizeChanged (int)),
            this, SLOT (slotSpraycanSizeChanged (int)));
    m_toolWidgetSpraycanSize->show ();

    kpToolFlowBase::begin ();
}

// public virtual [base kpToolFlowBase]
void kpToolSpraycan::end ()
{
    kpToolFlowBase::end ();

    disconnect (m_toolWidgetSpraycanSize, SIGNAL (spraycanSizeChanged (int)),
                this, SLOT (slotSpraycanSizeChanged (int)));
    m_toolWidgetSpraycanSize = 0;
}


// protected
void kpToolSpraycan::paintersSprayOneDocPoint (QPainter *painter,
    QPainter *maskPainter,
    const QRect &docRect,
    const QPoint &docPoint)
{
    QPolygon pArray (10);
    int numPointsCreated = 0;

#if DEBUG_KP_TOOL_SPRAYCAN
    kDebug () << "\t\tkpToolSpraycan::paintersSprayOneDocPoint(docRect=" << docRect
               << ",docPoint=" << docPoint
               << ") spraycanSize=" << spraycanSize ()
               << endl;
#endif

    const int radius = spraycanSize () / 2;

    for (int i = 0; i < 10; i++)
    {
        int dx, dy;

        dx = (rand () % spraycanSize ()) - radius;
        dy = (rand () % spraycanSize ()) - radius;

        // make it look circular
        // OPT: can be done better
        if (dx * dx + dy * dy <= radius * radius)
            pArray [numPointsCreated++] = QPoint (docPoint.x () + dx, docPoint.y () + dy);
    }

    pArray.resize (numPointsCreated);

#if DEBUG_KP_TOOL_SPRAYCAN && 0
    kDebug () << "\t\t\tnumPointsCreated=" << numPointsCreated << endl;
#endif


    if (numPointsCreated == 0)
        return;
        
    
    for (int i = 0; i < numPointsCreated; i++)
    {
    #if DEBUG_KP_TOOL_SPRAYCAN && 0
        kDebug () << "\t\t\t\t" << i << ": " << pArray [i] << endl;
    #endif
    
        QPoint pt (pArray [i].x () - docRect.x (),
                   pArray [i].y () - docRect.y ());

        if (painter->isActive ())
            painter->drawPoint (pt);

        if (maskPainter->isActive ())
            maskPainter->drawPoint (pt);
    }
}

// protected
void kpToolSpraycan::pixmapSprayManyDocPoints (QPixmap *pixmap,
    const QRect &docRect,
    const QList <QPoint> &docPoints)
{
#if DEBUG_KP_TOOL_SPRAYCAN
    kDebug () << "\tkpToolSpraycan::pixmapSprayManyDocPoints(docRect="
               << docRect << ")"
               << endl;
#endif

    QBitmap maskBitmap;
    QPainter painter, maskPainter;
    
    drawLineSetupPainterMask (pixmap,
        &maskBitmap,
        &painter, &maskPainter);
        
    for (QList <QPoint>::const_iterator pit = docPoints.begin ();
         pit != docPoints.end ();
         pit++)
    {
        paintersSprayOneDocPoint (&painter, &maskPainter,
            docRect, *pit);
    }

    drawLineTearDownPainterMask (pixmap,
        &maskBitmap,
        &painter, &maskPainter);
}


// public virtual [base kpToolFlowBase]
void kpToolSpraycan::beginDraw ()
{
#if DEBUG_KP_TOOL_SPRAYCAN
    kDebug () << "kpToolSpraycan::beginDraw()" << endl;
#endif

    kpToolFlowBase::beginDraw ();
    
    // use a timer instead of reimplementing draw() (we don't draw all the time)
    m_timer->start (25);    
}


// protected
QRect kpToolSpraycan::drawLineWithProbability (const QPoint &thisPoint,
         const QPoint &lastPoint,
         double probability)
{
#if DEBUG_KP_TOOL_SPRAYCAN
    kDebug () << "kpToolSpraycan::drawLine(thisPoint=" << thisPoint
               << ",lastPoint=" << lastPoint
               << ")" << endl;
#endif

    QRect docRect = kpBug::QRect_Normalized (QRect (thisPoint, lastPoint));
    docRect = neededRect (docRect, spraycanSize ());
    QPixmap pixmap = document ()->getPixmapAt (docRect);

    
    QList <QPoint> docPoints = interpolatePoints (thisPoint, lastPoint,
        probability);
    
        
    // Drawing a line (not just a point) starting at lastPoint?
    if (thisPoint != lastPoint &&
        docPoints.size () > 0 && docPoints [0] == lastPoint)
    {
    #if DEBUG_KP_TOOL_SPRAYCAN
        kDebug () << "\tis a line starting at lastPoint - erasing="
                   << docPoints [0] << endl;
    #endif
    
        // We're not expecting a duplicate 2nd interpolation point.
        Q_ASSERT (docPoints.size () <= 1 || docPoints [1] != lastPoint);
        
        // lastPoint was drawn previously so don't draw over it again or
        // it will (theoretically) be denser than expected.
        //
        // Unlike other tools such as the Brush, drawing over the same
        // point does result in a different appearance.
        //
        // Having said this, the user probably won't notice either way
        // since spraying on nearby document interpolation points will
        // spray around this document point anyway (due to the
        // spraycanSize() radius).
        // TODO: what if docPoints becomes empty?
        docPoints.erase (docPoints.begin ());
    }

    // By chance no points to draw?
    if (docPoints.empty ())
        return QRect ();
    
        
    pixmapSprayManyDocPoints (&pixmap, docRect, docPoints);

        
    viewManager ()->setFastUpdates ();
    document ()->setPixmapAt (pixmap, docRect.topLeft ());
    viewManager ()->restoreFastUpdates ();

    return docRect;
}

// public virtual [base kpToolFlowBase]
QRect kpToolSpraycan::drawPoint (const QPoint &point)
{
#if DEBUG_KP_TOOL_SPRAYCAN
    kDebug () << "kpToolSpraycan::drawPoint" << point
               << " lastPoint=" << m_lastPoint
               << endl;
#endif

    // if this is the first in the flow or if the user is moving the spray, make the spray line continuous
    if (point != m_lastPoint)
    {
        // without delay
        return drawLineWithProbability (point, point,
            1.0/*100% chance of drawing*/);
    }
    
    return QRect ();
}
    
// public virtual [base kpToolFlowBase]
QRect kpToolSpraycan::drawLine (const QPoint &thisPoint, const QPoint &lastPoint)
{
    return drawLineWithProbability (thisPoint, lastPoint,
        0.1/*less dense: select 10% of adjacent pixels - not all*/);
}

void kpToolSpraycan::timeoutDraw ()
{
#if DEBUG_KP_TOOL_SPRAYCAN
    kDebug () << "kpToolSpraycan::timeoutDraw()" << endl;
#endif

    const QRect drawnRect = drawLineWithProbability (m_currentPoint, m_currentPoint,
        1.0/*100% chance of drawing*/);

    m_currentCommand->updateBoundingRect (drawnRect);
}

    
// public virtual [base kpToolFlowBase]
void kpToolSpraycan::cancelShape ()
{
#if DEBUG_KP_TOOL_SPRAYCAN
    kDebug () << "kpToolSpraycan::cancelShape()" << endl;
#endif

    m_timer->stop ();
    kpToolFlowBase::cancelShape ();
}

// public virtual [base kpToolFlowBase]
void kpToolSpraycan::endDraw (const QPoint &thisPoint,
    const QRect &normalizedRect)
{
#if DEBUG_KP_TOOL_SPRAYCAN
    kDebug () << "kpToolSpraycan::endDraw(thisPoint=" << thisPoint
               << ")" << endl;
#endif

    m_timer->stop ();
    kpToolFlowBase::endDraw (thisPoint, normalizedRect);
}


// protected
int kpToolSpraycan::spraycanSize () const
{
    return m_toolWidgetSpraycanSize->spraycanSize ();
}

// protected slot
void kpToolSpraycan::slotSpraycanSizeChanged (int size)
{
    (void) size;
}


#include <kptoolspraycan.moc>

