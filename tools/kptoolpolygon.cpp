
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
#include <kptoolpolygon.h>
#include <kptooltoolbar.h>
#include <kptoolwidgetfillstyle.h>
#include <kptoolwidgetlinestyle.h>
#include <kptoolwidgetlinewidth.h>
#include <kpviewmanager.h>

#define DEBUG_KPTOOL_LINE 1

static const char *pointArrayToString (const QPointArray &pointArray)
{
    static char string [1000];
    string [0] = '\0';
    
    for (QPointArray::ConstIterator it = pointArray.begin ();
         it != pointArray.end ();
         it++)
    {
        QString ps = QString (" (%1, %2)").arg ((*it).x ()).arg ((*it).y ());
        const char *pss = ps.latin1 ();
        if (strlen (string) + strlen (pss) + 1 > sizeof (string) / sizeof (string [0]))
            break;
        strcat (string, pss);
    }
    
    return string;
}

static QPixmap pixmap (const QPixmap &oldPixmap,
                       const QPointArray &points, const QRect &rect,
                       const QPen &pen, const QBrush &brush,
                       enum kpToolPolygon::Mode mode, bool final = true)
{
    QPixmap pixmap = oldPixmap;

#if DEBUG_KPTOOL_LINE
    kdDebug () << "kptoolpolygon.cpp: pixmap(): points=" << pointArrayToString (points) << endl;
#endif

    QPointArray pointsInRect = points;
    pointsInRect.detach ();
    pointsInRect.translate (-rect.x (), -rect.y ());
    
    QPainter painter (&pixmap);
    painter.setPen (pen);
    painter.setBrush (brush);
    if (mode != kpToolPolygon::Polygon)
        painter.drawPolyline (pointsInRect);
    else
    {
        // TODO: why aren't the ends rounded?
        painter.drawPolygon (pointsInRect);
        if (!final)
        {
            int count = pointsInRect.count ();
            
            if (count > 2)
            {
                QPen XORpen = pen;
                XORpen.setColor (Qt::white);
                
                painter.setPen (XORpen);
                painter.setRasterOp (Qt::XorROP);
            
                painter.drawLine (pointsInRect [0], pointsInRect [count - 1]);
            }
        }
    }
    painter.end ();

    return pixmap;
}


/*
 * kpToolPolygon
 */

kpToolPolygon::kpToolPolygon (kpMainWindow *mainWindow)
    : kpTool (i18n ("Polygon"), i18n ("Draws polygons"), mainWindow, "tool_polygon"),
      m_mode (Polygon),
      m_toolWidgetFillStyle (0), m_toolWidgetLineStyle (0), m_toolWidgetLineWidth (0)
{
}

kpToolPolygon::~kpToolPolygon ()
{
}

void kpToolPolygon::setMode (Mode m)
{
    m_mode = m;
}


// virtual
void kpToolPolygon::begin ()
{
    kpToolToolBar *tb = toolToolBar ();

    kdDebug () << "kpToolPolygon::begin() tb=" << tb << endl;

    if (tb)
    {
        if (m_mode == Polygon)
            m_toolWidgetFillStyle = tb->toolWidgetFillStyle ();
        else
            m_toolWidgetFillStyle = 0;

        m_toolWidgetLineStyle = tb->toolWidgetLineStyle ();
        m_toolWidgetLineWidth = tb->toolWidgetLineWidth ();

        if (m_toolWidgetFillStyle)
        {
            connect (m_toolWidgetFillStyle, SIGNAL (fillStyleChanged (Qt::BrushStyle)),
                     this, SLOT (slotFillStyleChanged (Qt::BrushStyle)));
        }
        connect (m_toolWidgetLineStyle, SIGNAL (lineStyleChanged (Qt::PenStyle)),
                 this, SLOT (slotLineStyleChanged (Qt::PenStyle)));
        connect (m_toolWidgetLineWidth, SIGNAL (lineWidthChanged (int)),
                 this, SLOT (slotLineWidthChanged (int)));

        if (m_toolWidgetFillStyle)
            m_toolWidgetFillStyle->show ();
        m_toolWidgetLineStyle->show ();
        m_toolWidgetLineWidth->show ();

        if (m_toolWidgetFillStyle)
            m_fillStyle = m_toolWidgetFillStyle->fillStyle ();
        else
            m_fillStyle = Qt::NoBrush;
        m_lineStyle = m_toolWidgetLineStyle->lineStyle ();
        m_lineWidth = m_toolWidgetLineWidth->lineWidth ();
    }
    else
    {
        m_toolWidgetFillStyle = 0;
        m_toolWidgetLineStyle = 0;
        m_toolWidgetLineWidth = 0;

        m_fillStyle = Qt::NoBrush;
        m_lineStyle = Qt::SolidLine;
        m_lineWidth = 1;
    }

    viewManager ()->setCursor (QCursor (CrossCursor));
    
    m_originatingMouseButton = -1;
}

// virtual
void kpToolPolygon::end ()
{
    endShape ();

    if (m_toolWidgetFillStyle)
    {
        disconnect (m_toolWidgetFillStyle, SIGNAL (fillStyleChanged (Qt::BrushStyle)),
                    this, SLOT (slotFillStyleChanged (Qt::BrushStyle)));
        m_toolWidgetFillStyle = 0;
    }
    
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


void kpToolPolygon::beginDraw ()
{
#if DEBUG_KPTOOL_LINE
    kdDebug () << "kpToolPolygon::beginDraw()  m_points=" << pointArrayToString (m_points)
               << ", startPoint=" << m_startPoint << endl;
#endif

    // starting with a line...    
    if (m_points.count () == 0)
    {
        m_originatingMouseButton = m_mouseButton;
        m_points.putPoints (m_points.count (), 2,
                            m_startPoint.x (), m_startPoint.y (),
                            m_startPoint.x (), m_startPoint.y ());
    }
    // continuing poly*
    else
    {
        if (m_mouseButton != m_originatingMouseButton)
        {
            m_mouseButton = m_originatingMouseButton;
            endShape ();
        }
        else
        {
            int count = m_points.count ();
            m_points.putPoints (count, 1,
                                m_startPoint.x (), m_startPoint.y ());

            // start point = last end point;
            // _not_ the new/current start point
            // (which is disregarded in a poly* as only the end points count
            //  after the initial line)
            m_startPoint = m_points [count - 1];
        }
    }
                        
#if DEBUG_KPTOOL_LINE
    kdDebug () << "\tafterwards, m_points=" << pointArrayToString (m_points) << endl;
#endif
}

// private
void kpToolPolygon::applyModifiers ()
{
    int count = m_points.count ();

    m_toolLineStartPoint = m_startPoint;  /* also correct for poly* tool (see beginDraw()) */
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
        if (count == 2)
            m_toolLineStartPoint += (m_toolLineStartPoint - m_toolLineEndPoint);
        else
            m_toolLineEndPoint += (m_toolLineEndPoint - m_toolLineStartPoint);
    }    // if (m_controlPressed) {

    m_points [count - 2] = m_toolLineStartPoint;
    m_points [count - 1] = m_toolLineEndPoint;
    
    m_toolLineRect = kpTool::neededRect (QRect (m_toolLineStartPoint, m_toolLineEndPoint).normalize (),
                                         m_lineWidth);
}

// virtual
void kpToolPolygon::draw (const QPoint &, const QPoint &, const QRect &)
{
    if (m_points.count () == 0)
        return;
        
#if DEBUG_KPTOOL_LINE
    kdDebug () << "kpToolPolygon::draw()  m_points=" << pointArrayToString (m_points)
               << ", endPoint=" << m_currentPoint << endl;
#endif    

    applyModifiers ();

#if DEBUG_KPTOOL_LINE
    kdDebug () << "\tafterwards, m_points=" << pointArrayToString (m_points) << endl;
#endif

    updateShape ();
    
    emit mouseDragged (QRect (m_toolLineStartPoint, m_toolLineEndPoint));
}

// private slot
void kpToolPolygon::updateShape ()
{
    if (m_points.count () == 0)
        return;

    QRect boundingRect = kpTool::neededRect (m_points.boundingRect (), m_lineWidth);
    
    QPixmap oldPixmap = document ()->getPixmapAt (boundingRect);
    QPixmap newPixmap = pixmap (oldPixmap,
                                m_points, boundingRect,
                                pen (), brush (),
                                m_mode, false/*not final*/);

    viewManager ()->setTempPixmapAt (newPixmap, boundingRect.topLeft ());
}

// virtual
void kpToolPolygon::cancelShape ()
{
#if 0
    endDraw (QPoint (), QRect ());
    commandHistory ()->undo ();
#else
    viewManager ()->invalidateTempPixmap ();
#endif
    m_points.resize (0);
}

// virtual
void kpToolPolygon::endDraw (const QPoint &, const QRect &)
{
#if DEBUG_KPTOOL_LINE
    kdDebug () << "kpToolPolygon::endDraw()  m_points=" << pointArrayToString (m_points) << endl;
#endif

    if (m_points.count () == 0)
        return;

    if (m_mode == Line || m_points.count () >= 50)
        endShape ();
}

// public virtual
void kpToolPolygon::endShape (const QPoint &, const QRect &)
{
#if DEBUG_KPTOOL_LINE
    kdDebug () << "kpToolPolygon::endShape()  m_points=" << pointArrayToString (m_points) << endl;
#endif

    if (!hasBegunShape ())
        return;
        
    viewManager ()->invalidateTempPixmap (true);

    QRect boundingRect = kpTool::neededRect (m_points.boundingRect (), m_lineWidth);
    
    kpToolPolygonCommand *lineCommand = new kpToolPolygonCommand
        (viewManager (), document (),
            text (),
            m_points, boundingRect,
            pen (), brush (),
            document ()->getPixmapAt (boundingRect),
            m_mode);

    commandHistory ()->addCommand (lineCommand);
    
    m_points.resize (0);
}

// public virtual
bool kpToolPolygon::hasBegunShape () const
{
    return (m_points.count () > 0);
}


// public slot
void kpToolPolygon::slotLineStyleChanged (Qt::PenStyle lineStyle)
{
    m_lineStyle = lineStyle;
    updateShape ();
}

// public slot
void kpToolPolygon::slotLineWidthChanged (int width)
{
    m_lineWidth = width;
    updateShape ();
}

// public slot
void kpToolPolygon::slotFillStyleChanged (Qt::BrushStyle fillStyle)
{
    m_fillStyle = fillStyle;
    updateShape ();
}

// virtual protected slot
void kpToolPolygon::slotForegroundColorChanged (const QColor &)
{
    updateShape ();
}

// virtual protected slot
void kpToolPolygon::slotBackgroundColorChanged (const QColor &)
{
    updateShape ();
}


// private
QPen kpToolPolygon::pen () const
{
    return QPen (color (m_mouseButton),
                 m_lineWidth, m_lineStyle,
                 Qt::RoundCap, Qt::RoundJoin);
}

// private
QBrush kpToolPolygon::brush () const
{
    return QBrush (color (1 - m_mouseButton), m_fillStyle);
}


/*
 * kpToolPolygonCommand
 */

kpToolPolygonCommand::kpToolPolygonCommand (kpViewManager *viewManager, kpDocument *document,
                                            const QString &toolText,
                                            const QPointArray &points,
                                            const QRect &normalizedRect,
                                            const QPen &pen, const QBrush &brush,
                                            const QPixmap &originalArea,
                                            enum kpToolPolygon::Mode mode)
    : m_viewManager (viewManager), m_document (document),
      m_name (toolText),
      m_points (points),
      m_normalizedRect (normalizedRect),
      m_pen (pen), m_brush (brush),
      m_originalArea (originalArea),
      m_mode (mode)
{
    m_points.detach ();
}

kpToolPolygonCommand::~kpToolPolygonCommand ()
{
}

void kpToolPolygonCommand::execute ()
{
    QPixmap p = pixmap (m_originalArea,
                        m_points, m_normalizedRect,
                        m_pen, m_brush,
                        m_mode);
    m_document->setPixmapAt (p, m_normalizedRect.topLeft ());
}

void kpToolPolygonCommand::unexecute ()
{
    m_document->setPixmapAt (m_originalArea, m_normalizedRect.topLeft ());
}

QString kpToolPolygonCommand::name () const
{
    return m_name;
}

#include <kptoolpolygon.moc>
