
/* This file is part of the KolourPaint project
   Copyright (c) 2003 Clarence Dang <dang@kde.org>
   All rights reserved.
   
   Redistribution and use in source and binary forms, with or without
   modification, are permitted provided that the following conditions
   are met:
   
   1. Redistributions of source code must retain the above copyright
      notice, this list of conditions and the following disclaimer.
   2. Redistributions in binary form must reproduce the above copyright
      notice, this list of conditions and the following disclaimer in the
      documentation and/or other materials provided with the distribution.
   3. Neither the names of the copyright holders nor the names of
      contributors may be used to endorse or promote products derived from
      this software without specific prior written permission.
   
   THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
   "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
   LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A
   PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
   HOLDERS OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
   SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
   LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
   DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
   THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
   (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
   OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include <stdlib.h>

#include <qpainter.h>
#include <qpen.h>
#include <qpixmap.h>
#include <qpoint.h>
#include <qpointarray.h>
#include <qrect.h>
#include <qtimer.h>

#include <kdebug.h>
#include <klocale.h>

#include <kpdefs.h>
#include <kpdocument.h>
#include <kpmainwindow.h>
#include <kptoolairspray.h>
#include <kpview.h>
#include <kpviewmanager.h>


/*
 * kpToolAirSpray
 */

kpToolAirSpray::kpToolAirSpray (kpMainWindow *mainWindow)
    : kpTool (i18n ("Spraycan"), i18n ("Sprays graffiti"), mainWindow, "tool_spraycan"),
      m_currentCommand (0),
      m_radius (5)
{
    m_timer = new QTimer (this);
    connect (m_timer, SIGNAL (timeout ()), this, SLOT (actuallyDraw ()));
}

kpToolAirSpray::~kpToolAirSpray ()
{
    delete m_currentCommand;
}


void kpToolAirSpray::beginDraw ()
{
    m_currentCommand = new kpToolAirSprayCommand (document (), viewManager (),
                                                  QPen (color (m_mouseButton)));

    // without delay
    actuallyDraw ();

    // use a timer instead of reimplementing draw() (we don't draw all the time)
    m_timer->start (25);
}

void kpToolAirSpray::draw (const QPoint &thisPoint, const QPoint &, const QRect &)
{
    emit mouseMoved (thisPoint);
}

void kpToolAirSpray::actuallyDraw ()
{
    QPointArray pArray (10);
    QPoint p = m_currentPoint;
    for (int i = 0; i < 10; i++)
    {
        int dx, dy;

        dx = (rand () % (2 * m_radius)) - m_radius;
        dy = (rand () % (2 * m_radius)) - m_radius;

        // make it look circular
        // OPT: can be done better
        if (dx * dx + dy * dy <= m_radius * m_radius)
            pArray.setPoint (i, p.x () + dx, p.y () + dy);
    }

    // leave the command to draw
    // TODO: I bet you have a one-off error there
    m_currentCommand->addPoints (QRect (p.x () - m_radius, p.y () - m_radius,
                                        2 * m_radius, 2 * m_radius), pArray);
}

// virtual
void kpToolAirSpray::cancelDraw ()
{
#if 0
    endDraw (QPoint (), QRect ());
    mainWindow ()->commandHistory ()->undo ();
#else
    m_timer->stop ();

    m_currentCommand->cancel ();

    delete m_currentCommand;
    m_currentCommand = 0;
#endif
}

// virtual
void kpToolAirSpray::endDraw (const QPoint &, const QRect &)
{
    m_timer->stop ();

    m_currentCommand->finalize ();
    mainWindow ()->commandHistory ()->addCommand (m_currentCommand, false /* don't exec */);

    // don't delete - it's up to the commandHistory
    m_currentCommand = 0;
}


/*
 * kpToolAirSprayCommand
 */

kpToolAirSprayCommand::kpToolAirSprayCommand (kpDocument *document, kpViewManager *viewManager,
                                              const QPen &pen)
    : m_document (document),
      m_viewManager (viewManager),
      m_pen (pen),
      m_newPixmapPtr (0)
{
    m_oldPixmap = *document->pixmap ();
}

kpToolAirSprayCommand::~kpToolAirSprayCommand ()
{
    delete m_newPixmapPtr;
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
        m_document->setPixmapAt (*m_newPixmapPtr, m_boundingRect.topLeft ());

        // (will be regenerated in unexecute() if required)
        delete m_newPixmapPtr;
        m_newPixmapPtr = 0;
    }
    else
        kdError (KP_AREA) << "kpToolAirSprayCommand::execute() has null m_newPixmapPtr" << endl;
}

// Undo:
void kpToolAirSprayCommand::unexecute ()
{
    if (!m_newPixmapPtr)
    {
        // the ultimate in laziness - figure out Redo info only if we Undo
        m_newPixmapPtr = new QPixmap (m_boundingRect.width (), m_boundingRect.height ());
        *m_newPixmapPtr = m_document->getPixmapAt (m_boundingRect);
    }
    else
        kdError (KP_AREA) << "kpToolAirSprayCommand::unexecute() has non-null newPixmapPtr" << endl;

    m_document->setPixmapAt (m_oldPixmap, m_boundingRect.topLeft ());
}

// virtual
QString kpToolAirSprayCommand::name () const
{
    return i18n ("Air Spray");
}

void kpToolAirSprayCommand::addPoints (const QRect &docRect, const QPointArray &points)
{
    QPixmap pixmap = m_document->getPixmapAt (docRect);
    QPainter painter;

    painter.begin (&pixmap);
    painter.setPen (m_pen);
    for (int i = 0; i < (int) points.count (); i++)
    {
        painter.drawPoint (points [i].x () - docRect.x (),
                           points [i].y () - docRect.y ());
    }
    painter.end ();

    m_document->setPixmapAt (pixmap, docRect.topLeft ());

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
        m_document->setPixmapAt (m_oldPixmap, QPoint (0, 0));
    }
}

#include <kptoolairspray.moc>
