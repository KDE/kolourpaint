
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

#include <math.h>

#include <qcursor.h>
#include <qlayout.h>
#include <qpainter.h>
#include <qpoint.h>
#include <qpushbutton.h>
#include <qrect.h>
#include <qtooltip.h>
#include <qvbuttongroup.h>

#include <kdebug.h>
#include <klocale.h>

#include <kpdocument.h>
#include <kpdefs.h>
#include <kpmainwindow.h>
#include <kptoolline.h>
#include <kptooltoolbar.h>
#include <kptoolwidgetlinestyle.h>
#include <kptoolwidgetlinewidth.h>
#include <kpviewmanager.h>

/*
 * kpToolLine
 */

kpToolLine::kpToolLine (kpMainWindow *mainWindow)
    : kpTool (i18n ("Line"), i18n ("Draws lines"), mainWindow, "tool_line")
{
}

kpToolLine::~kpToolLine ()
{
}

// virtual
void kpToolLine::begin ()
{
    kpToolToolBar *tb = toolToolBar ();

    kdDebug () << "kpToolLine::begin() tb=" << tb << endl;

    if (tb)
    {
        m_toolWidgetLineStyle = tb->toolWidgetLineStyle ();
        m_toolWidgetLineWidth = tb->toolWidgetLineWidth ();

        connect (m_toolWidgetLineStyle, SIGNAL (lineStyleChanged (Qt::PenStyle)),
                 this, SLOT (slotLineStyleChanged (Qt::PenStyle)));
        connect (m_toolWidgetLineWidth, SIGNAL (lineWidthChanged (int)),
                 this, SLOT (slotLineWidthChanged (int)));

        m_toolWidgetLineStyle->show ();
        m_toolWidgetLineWidth->show ();

        m_lineStyle = m_toolWidgetLineStyle->lineStyle ();
        m_lineWidth = m_toolWidgetLineWidth->lineWidth ();
    }
    else
    {
        m_toolWidgetLineStyle = 0;
        m_toolWidgetLineWidth = 0;

        m_lineStyle = Qt::SolidLine;
        m_lineWidth = 1;
    }

    viewManager ()->setCursor (QCursor (CrossCursor));
}

// virtual
void kpToolLine::end ()
{
    if (m_toolWidgetLineWidth)
    {
        disconnect (m_toolWidgetLineWidth, SIGNAL (lineWidthChanged (int)),
                    this, SLOT (slotLineWidthChanged (int)));
        m_toolWidgetLineWidth = 0;
    }

    if (m_toolWidgetLineStyle)
    {
        disconnect (m_toolWidgetLineStyle, SIGNAL (lineStyleChanged (Qt::PenStyle)),
                    this, SLOT (slotLineStyleChanged (Qt::PenStyle)));
        m_toolWidgetLineStyle = 0;
    }

    viewManager ()->unsetCursor ();
}

void kpToolLine::beginDraw ()
{
}

static QPixmap pixmap (const QPixmap &oldPixmap,
                       const QPoint &startPoint, const QPoint &point, const QRect &rect,
                       const QPen &pen)
{
    QPixmap pixmap = oldPixmap;

    QPainter painter (&pixmap);
    painter.setPen (pen);
    painter.drawLine (startPoint - rect.topLeft (), point - rect.topLeft ());

    return pixmap;
}

// private
void kpToolLine::applyModifiers ()
{
    m_toolLineStartPoint = m_startPoint;
    m_toolLineEndPoint = m_currentPoint;

    // angles
    if (m_shiftPressed || m_altPressed)
    {
        int diffx = m_toolLineEndPoint.x () - m_toolLineStartPoint.x ();
        int diffy = m_toolLineEndPoint.y () - m_toolLineStartPoint.y ();

        double ratio;
        if (fabs (diffx - 0) < KP_EPSILON)
            ratio = KP_DOCUMENT_MAX_HEIGHT * 16;  // where's DBL_MAX?
        else
            ratio = fabs (double (diffy) / double (diffx));

        // Shift        = 0, 45, 90
        // Alt          = 0, 30, 60, 90
        // Shift + Alt  = 0, 30, 45, 60, 90
        double angles [10];  // "ought to be enough for anybody"
        int numAngles = 0;
        angles [numAngles++] = 0;
        if (m_altPressed)
            angles [numAngles++] = KP_PI / 6;
        if (m_shiftPressed)
            angles [numAngles++] = KP_PI / 4;
        if (m_altPressed)
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

        // horizontal (dist from start !maintained)
        if (fabs (angle - 0) < KP_EPSILON)
            m_toolLineEndPoint = QPoint (m_toolLineEndPoint.x (), m_toolLineStartPoint.y ());
        // vertical (dist from start !maintained)
        else if (fabs (angle - KP_PI / 2) < KP_EPSILON)
            m_toolLineEndPoint = QPoint (m_toolLineStartPoint.x (), m_toolLineEndPoint.y ());
        // diagonal (dist from start maintained)
        else
        {
            double dist = sqrt (diffx * diffx + diffy * diffy);

            #define sgn(a) ((a)<0?-1:1)
            int dx = int (m_toolLineStartPoint.x () + dist * cos (angle) * sgn (diffx));
            int dy = int (m_toolLineStartPoint.y () + dist * sin (angle) * sgn (diffy));
            #undef sgn

            m_toolLineEndPoint = QPoint (dx, dy);
        }
    }    // if (m_shiftPressed || m_altPressed) {

    // centring
    if (m_controlPressed)
    {
        // start = start - diff
        //       = start - (end - start)
        //       = start - end + start
        //       = 2 * start - end
        m_toolLineStartPoint += (m_toolLineStartPoint - m_toolLineEndPoint);
    }    // if (m_controlPressed) {

    m_toolLineRect = kpTool::neededRect (QRect (m_toolLineStartPoint, m_toolLineEndPoint).normalize (),
                                         m_lineWidth);
}

// virtual
void kpToolLine::draw (const QPoint &, const QPoint &, const QRect &)
{
    applyModifiers ();

    QPixmap oldPixmap = document ()->getPixmapAt (m_toolLineRect);
    QPixmap newPixmap = pixmap (oldPixmap, m_toolLineStartPoint, m_toolLineEndPoint, m_toolLineRect, pen ());

    viewManager ()->setTempPixmapAt (newPixmap, m_toolLineRect.topLeft ());
    emit mouseDragged (QRect (m_toolLineStartPoint, m_toolLineEndPoint));
}

// virtual
void kpToolLine::cancelDraw ()
{
#if 0
    endDraw (QPoint (), QRect ());
    commandHistory ()->undo ();
#else
    viewManager ()->invalidateTempPixmap ();
#endif
}

// virtual
void kpToolLine::endDraw (const QPoint &, const QRect &)
{
    applyModifiers ();

    viewManager ()->invalidateTempPixmap (true);

    kpToolLineCommand *lineCommand = new kpToolLineCommand
        (viewManager (), document (),
         m_toolLineStartPoint, m_toolLineEndPoint, m_toolLineRect,
         pen (),
         document ()->getPixmapAt (m_toolLineRect));

    commandHistory ()->addCommand (lineCommand);
}

// public slot
void kpToolLine::slotLineStyleChanged (Qt::PenStyle lineStyle)
{
    m_lineStyle = lineStyle;
}

// public slot
void kpToolLine::slotLineWidthChanged (int width)
{
    m_lineWidth = width;
}

// private
QPen kpToolLine::pen () const
{
    return QPen (mainWindow ()->color (m_mouseButton),
                 m_lineWidth, m_lineStyle,
                 Qt::RoundCap, Qt::MiterJoin);
}

/*
 * kpToolLineCommand
 */

kpToolLineCommand::kpToolLineCommand (kpViewManager *viewManager, kpDocument *document,
                                        const QPoint &startPoint, const QPoint &endPoint,
                                        const QRect &normalizedRect,
                                        const QPen &pen,
                                        const QPixmap &originalArea)
    : m_viewManager (viewManager), m_document (document),
      m_startPoint (startPoint), m_endPoint (endPoint),
      m_normalizedRect (normalizedRect),
      m_pen (pen),
      m_originalArea (originalArea)
{
}

kpToolLineCommand::~kpToolLineCommand ()
{
}

void kpToolLineCommand::execute ()
{
    QPixmap p = pixmap (m_originalArea,
                        m_startPoint, m_endPoint, m_normalizedRect,
                        m_pen);
    m_document->setPixmapAt (p, m_normalizedRect.topLeft ());
}

void kpToolLineCommand::unexecute ()
{
    m_document->setPixmapAt (m_originalArea, m_normalizedRect.topLeft ());
}

QString kpToolLineCommand::name () const
{
    return i18n ("Line");
}

#include <kptoolline.moc>
