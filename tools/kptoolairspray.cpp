
/*
   Copyright (c) 2003,2004,2005 Clarence Dang <dang@kde.org>
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


#define DEBUG_KP_TOOL_SPRAYCAN 0

#include <stdlib.h>

#include <qbitmap.h>
#include <qpainter.h>
#include <qpen.h>
#include <qpixmap.h>
#include <qpoint.h>
#include <qpointarray.h>
#include <qrect.h>
#include <qtimer.h>

#include <kdebug.h>
#include <klocale.h>

#include <kpcommandhistory.h>
#include <kpdefs.h>
#include <kpdocument.h>
#include <kpmainwindow.h>
#include <kppixmapfx.h>
#include <kptoolairspray.h>
#include <kptooltoolbar.h>
#include <kptoolwidgetspraycansize.h>
#include <kpview.h>
#include <kpviewmanager.h>


/*
 * kpToolAirSpray
 */

kpToolAirSpray::kpToolAirSpray (kpMainWindow *mainWindow)
    : kpTool (i18n ("Spraycan"), i18n ("Sprays graffiti"),
              Qt::Key_Y,
              mainWindow, "tool_spraycan"),
      m_currentCommand (0)
{
    m_timer = new QTimer (this);
    connect (m_timer, SIGNAL (timeout ()), this, SLOT (actuallyDraw ()));
}

kpToolAirSpray::~kpToolAirSpray ()
{
    delete m_currentCommand;
}


// private
QString kpToolAirSpray::haventBegunDrawUserMessage () const
{
    return i18n ("Click or drag to spray graffiti.");
}

// public virtual
void kpToolAirSpray::begin ()
{
    kpToolToolBar *tb = toolToolBar ();

    m_toolWidgetSpraycanSize = 0;
    m_size = 10;

    if (tb)
    {
        m_toolWidgetSpraycanSize = tb->toolWidgetSpraycanSize ();

        if (m_toolWidgetSpraycanSize)
        {
            m_size = m_toolWidgetSpraycanSize->spraycanSize ();
            connect (m_toolWidgetSpraycanSize, SIGNAL (spraycanSizeChanged (int)),
                     this, SLOT (slotSpraycanSizeChanged (int)));

            m_toolWidgetSpraycanSize->show ();
        }
    }

    setUserMessage (haventBegunDrawUserMessage ());
}

// public virtual
void kpToolAirSpray::end ()
{
    if (m_toolWidgetSpraycanSize)
    {
        disconnect (m_toolWidgetSpraycanSize, SIGNAL (spraycanSizeChanged (int)),
                    this, SLOT (slotSpraycanSizeChanged (int)));
        m_toolWidgetSpraycanSize = 0;
    }

    setUserMessage (haventBegunDrawUserMessage ());
}

// private slot
void kpToolAirSpray::slotSpraycanSizeChanged (int size)
{
    m_size = size;
}


void kpToolAirSpray::beginDraw ()
{
    m_currentCommand = new kpToolAirSprayCommand (
        color (m_mouseButton),
        m_size,
        mainWindow ());

    // without delay
    actuallyDraw ();

    // use a timer instead of reimplementing draw() (we don't draw all the time)
    m_timer->start (25);

    setUserMessage (cancelUserMessage ());
}

void kpToolAirSpray::draw (const QPoint &thisPoint, const QPoint &, const QRect &)
{
    // if the user is moving the spray, make the spray line continuous
    if (thisPoint != m_lastPoint)
    {
        // without delay
        actuallyDraw ();
    }

    setUserShapePoints (thisPoint);
}

void kpToolAirSpray::actuallyDraw ()
{
    QPointArray pArray (10);
    int numPoints = 0;

    QPoint p = m_currentPoint;

#if DEBUG_KP_TOOL_SPRAYCAN
    kdDebug () << "kpToolAirSpray::actuallyDraw() currentPoint=" << p
               << " size=" << m_size
               << endl;
#endif

    int radius = m_size / 2;

    for (int i = 0; i < 10; i++)
    {
        int dx, dy;

        dx = (rand () % m_size) - radius;
        dy = (rand () % m_size) - radius;

        // make it look circular
        // OPT: can be done better
        if (dx * dx + dy * dy <= radius * radius)
            pArray [numPoints++] = QPoint (p.x () + dx, p.y () + dy);
    }

    pArray.resize (numPoints);

    if (numPoints > 0)
    {
        // leave the command to draw
        m_currentCommand->addPoints (pArray);
    }
}

// virtual
void kpToolAirSpray::cancelShape ()
{
#if 0
    endDraw (QPoint (), QRect ());
    mainWindow ()->commandHistory ()->undo ();
#else
    m_timer->stop ();

    m_currentCommand->finalize ();
    m_currentCommand->cancel ();

    delete m_currentCommand;
    m_currentCommand = 0;
#endif

    setUserMessage (i18n ("Let go of all the mouse buttons."));
}

void kpToolAirSpray::releasedAllButtons ()
{
    setUserMessage (haventBegunDrawUserMessage ());
}

// virtual
void kpToolAirSpray::endDraw (const QPoint &, const QRect &)
{
    m_timer->stop ();

    m_currentCommand->finalize ();
    mainWindow ()->commandHistory ()->addCommand (m_currentCommand, false /* don't exec */);

    // don't delete - it's up to the commandHistory
    m_currentCommand = 0;

    setUserMessage (haventBegunDrawUserMessage ());
}


/*
 * kpToolAirSprayCommand
 */

kpToolAirSprayCommand::kpToolAirSprayCommand (const kpColor &color, int size,
                                              kpMainWindow *mainWindow)
    : kpCommand (mainWindow),
      m_color (color),
      m_size (size),
      m_newPixmapPtr (0)
{
    m_oldPixmap = *document ()->pixmap ();
}

kpToolAirSprayCommand::~kpToolAirSprayCommand ()
{
    delete m_newPixmapPtr;
}


// public virtual [base kpCommand]
QString kpToolAirSprayCommand::name () const
{
    return i18n ("Spraycan");
}


// public virtual [base kpCommand]
int kpToolAirSprayCommand::size () const
{
    return kpPixmapFX::pixmapSize (m_newPixmapPtr) +
           kpPixmapFX::pixmapSize (m_oldPixmap);
}


// Redo:
//
// must not call before unexecute() as m_newPixmapPtr is null
// (one reason why we told addCommand() not to execute,
//  the other being that the dots have already been draw onto the doc)
void kpToolAirSprayCommand::execute ()
{
    if (m_newPixmapPtr)
    {
        document ()->setPixmapAt (*m_newPixmapPtr, m_boundingRect.topLeft ());

        // (will be regenerated in unexecute() if required)
        delete m_newPixmapPtr;
        m_newPixmapPtr = 0;
    }
    else
        kdError () << "kpToolAirSprayCommand::execute() has null m_newPixmapPtr" << endl;
}

// Undo:
void kpToolAirSprayCommand::unexecute ()
{
    if (!m_newPixmapPtr)
    {
        // the ultimate in laziness - figure out Redo info only if we Undo
        m_newPixmapPtr = new QPixmap (m_boundingRect.width (), m_boundingRect.height ());
        *m_newPixmapPtr = document ()->getPixmapAt (m_boundingRect);
    }
    else
        kdError () << "kpToolAirSprayCommand::unexecute() has non-null newPixmapPtr" << endl;

    document ()->setPixmapAt (m_oldPixmap, m_boundingRect.topLeft ());
}


// public
void kpToolAirSprayCommand::addPoints (const QPointArray &points)
{
    QRect docRect = points.boundingRect ();

#if DEBUG_KP_TOOL_SPRAYCAN
    kdDebug () << "kpToolAirSprayCommand::addPoints() docRect=" << docRect
               << " numPoints=" << points.count () << endl;
    for (int i = 0; i < (int) points.count (); i++)
        kdDebug () << "\t" << i << ": " << points [i] << endl;
#endif

    QPixmap pixmap = document ()->getPixmapAt (docRect);
    QBitmap mask;

    QPainter painter, maskPainter;

    if (m_color.isOpaque ())
    {
        painter.begin (&pixmap);
        painter.setPen (m_color.toQColor ());
    }

    if (pixmap.mask () || m_color.isTransparent ())
    {
        mask = kpPixmapFX::getNonNullMask (pixmap);
        maskPainter.begin (&mask);
        maskPainter.setPen (m_color.maskColor ());
    }

    for (int i = 0; i < (int) points.count (); i++)
    {
        QPoint pt (points [i].x () - docRect.x (),
                   points [i].y () - docRect.y ());

        if (painter.isActive ())
            painter.drawPoint (pt);

        if (maskPainter.isActive ())
            maskPainter.drawPoint (pt);
    }

    if (maskPainter.isActive ())
        maskPainter.end ();

    if (painter.isActive ())
        painter.end ();

    if (!mask.isNull ())
        pixmap.setMask (mask);

    viewManager ()->setFastUpdates ();
    document ()->setPixmapAt (pixmap, docRect.topLeft ());
    viewManager ()->restoreFastUpdates ();

    m_boundingRect = m_boundingRect.unite (docRect);
}

void kpToolAirSprayCommand::finalize ()
{
    // store only needed part of doc pixmap
    m_oldPixmap = kpTool::neededPixmap (m_oldPixmap, m_boundingRect);
}

void kpToolAirSprayCommand::cancel ()
{
    if (m_boundingRect.isValid ())
    {
        viewManager ()->setFastUpdates ();
        document ()->setPixmapAt (m_oldPixmap, m_boundingRect.topLeft ());
        viewManager ()->restoreFastUpdates ();
    }
}

#include <kptoolairspray.moc>
