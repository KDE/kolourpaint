
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

#include <qcursor.h>
#include <qevent.h>
#include <qpainter.h>

#include <kdebug.h>
#include <klocale.h>

#include <kpdefs.h>
#include <kpdocument.h>
#include <kpmainwindow.h>
#include <kptoolrectangle.h>
#include <kptooltoolbar.h>
#include <kptoolwidgetlinestyle.h>
#include <kptoolwidgetlinewidth.h>
#include <kptoolwidgetfillstyle.h>
#include <kpview.h>
#include <kpviewmanager.h>

#define DEBUG_KPTOOLRECTANGLE 1

static QPixmap pixmap (const kpToolRectangle::Mode mode,
                       kpDocument *document, const QRect &rect,
                       const QPoint &startPoint, const QPoint &endPoint,
                       const QPen &pen, const QBrush &brush)
{
    QPixmap pixmap = document->getPixmapAt (rect);

    QPainter painter (&pixmap);
    painter.setPen (pen);
    painter.setBrush (brush);

    switch (mode)
    {
    case kpToolRectangle::Rectangle:
        painter.drawRect (QRect (startPoint - rect.topLeft (), endPoint - rect.topLeft ()));
        break;
    case kpToolRectangle::RoundedRectangle:
        painter.drawRoundRect (QRect (startPoint - rect.topLeft (), endPoint - rect.topLeft ()));
        break;
    case kpToolRectangle::Ellipse:
        painter.drawEllipse (QRect (startPoint - rect.topLeft (), endPoint - rect.topLeft ()));
        break;
    default:
        kdError () << "kptoolrectangle.cpp::pixmap() passed unknown mode: " << int (mode) << endl;
        break;
    }

    return pixmap;
}


/*
 * kpToolRectangle
 */

kpToolRectangle::kpToolRectangle (kpMainWindow *mainWindow)
    : kpTool (i18n ("Rectangle"), i18n ("Draws rectangles and squares"),
      mainWindow, "tool_rectangle"),
      m_mode (Rectangle),
      m_toolWidgetLineStyle (0),
      m_toolWidgetLineWidth (0)
{
}

kpToolRectangle::~kpToolRectangle ()
{
}

void kpToolRectangle::setMode (Mode mode)
{
    m_mode = mode;
}


// private
void kpToolRectangle::updatePens ()
{
    for (int i = 0; i < 2; i++)
        m_pen [i] = pen (i);
}

// private
void kpToolRectangle::updateBrushes ()
{
    for (int i = 0; i < 2; i++)
        m_brush [i] = brush (i);
}

// virtual private slot
void kpToolRectangle::slotForegroundColorChanged (const QColor &)
{
#if DEBUG_KPTOOLRECTANGLE
    kdDebug () << "kpToolRectangle::slotForegroundColorChanged()" << endl;
#endif
    m_pen [0] = pen (0);
    m_brush [1] = brush (1);
}

// virtual private slot
void kpToolRectangle::slotBackgroundColorChanged (const QColor &)
{
#if DEBUG_KPTOOLRECTANGLE
    kdDebug () << "kpToolRectangle::slotBackgroundColorChanged()" << endl;
    kdDebug () << "\tm_toolWidgetFillStyle=" << m_toolWidgetFillStyle << endl;
#endif
    m_pen [1] = pen (1);
    m_brush [0] = brush (0);
}

// private
QPen kpToolRectangle::pen (int mouseButton) const
{
    if (!m_toolWidgetLineWidth || !m_toolWidgetLineStyle)
        return QPen (color (mouseButton));
    
    return QPen (color (mouseButton),
                 m_toolWidgetLineWidth->lineWidth (),
                 m_toolWidgetLineStyle->lineStyle ());
}

QBrush kpToolRectangle::brush (int mouseButton) const
{
#if DEBUG_KPTOOLRECTANGLE
    kdDebug () << "kpToolRectangle::brush ()  mouseButton=" << mouseButton
               << " m_toolWidgetFillStyle=" << m_toolWidgetFillStyle
               << endl;
#endif
    return QBrush (color (1 - mouseButton),
                   m_toolWidgetFillStyle->fillStyle ());
}


// virtual
void kpToolRectangle::begin ()
{
#if DEBUG_KPTOOLRECTANGLE
    kdDebug () << "kpToolRectangle::begin ()" << endl;
#endif
    
    kpToolToolBar *tb = toolToolBar ();

#if DEBUG_KPTOOLRECTANGLE
    kdDebug () << "\ttoolToolBar=" << tb << endl;
#endif

    if (tb)
    {
        m_toolWidgetLineStyle = tb->toolWidgetLineStyle ();
        connect (m_toolWidgetLineStyle, SIGNAL (lineStyleChanged (Qt::PenStyle)),
                 this, SLOT (updatePens ()));
        m_toolWidgetLineStyle->show ();
                 
        m_toolWidgetLineWidth = tb->toolWidgetLineWidth ();
        connect (m_toolWidgetLineWidth, SIGNAL (lineWidthChanged (int)),
                 this, SLOT (updatePens ()));
        m_toolWidgetLineWidth->show ();
        
        updatePens ();

        
        m_toolWidgetFillStyle = tb->toolWidgetFillStyle ();
        connect (m_toolWidgetFillStyle, SIGNAL (fillStyleChanged (Qt::BrushStyle)),
                 this, SLOT (updateBrushes ()));
        m_toolWidgetFillStyle->show ();

        updateBrushes ();
    }
    
#if DEBUG_KPTOOLRECTANGLE
    kdDebug () << "\t\tm_toolWidgetFillStyle=" << m_toolWidgetFillStyle << endl;
#endif
    
    viewManager ()->setCursor (QCursor (CrossCursor));
}

// virtual
void kpToolRectangle::end ()
{
#if DEBUG_KPTOOLRECTANGLE
    kdDebug () << "kpToolRectangle::end ()" << endl;
#endif

    if (m_toolWidgetLineStyle)
    {
        disconnect (m_toolWidgetLineStyle, SIGNAL (lineStyleChanged (Qt::PenStyle)),
                    this, SLOT (updatePens ()));
        m_toolWidgetLineStyle = 0;
    }
    
    if (m_toolWidgetLineWidth)
    {
        disconnect (m_toolWidgetLineWidth, SIGNAL (lineWidthChanged (int)),
                    this, SLOT (updatePens ()));
        m_toolWidgetLineWidth = 0;
    }

    if (m_toolWidgetFillStyle)
    {
        disconnect (m_toolWidgetFillStyle, SIGNAL (fillStyleChanged (Qt::BrushStyle)),
                   this, SLOT (updateBrushes ()));
        m_toolWidgetFillStyle = 0;
    }

    viewManager ()->unsetCursor ();
}

void kpToolRectangle::applyModifiers ()
{
    QRect rect = QRect (m_startPoint, m_currentPoint).normalize ();

#if DEBUG_KPTOOLRECTANGLE
    kdDebug () << "kpToolRectangle::applyModifiers(" << rect
               << ") shift=" << m_shiftPressed
               << " ctrl=" << m_controlPressed
               << endl;
#endif

    // user wants to m_startPoint == centre
    if (m_controlPressed)
    {
        int xdiff = kAbs (m_startPoint.x () - m_currentPoint.x ());
        int ydiff = kAbs (m_startPoint.y () - m_currentPoint.y ());
        rect = QRect (m_startPoint.x () - xdiff, m_startPoint.y () - ydiff,
                      xdiff * 2 + 1, ydiff * 2 + 1);
    }

    // user wants major axis == minor axis:
    //   rectangle --> square
    //   rounded rectangle --> rounded square
    //   ellipse --> circle
    if (m_shiftPressed)
    {
        if (!m_controlPressed)
        {
            if (rect.width () < rect.height ())
            {
                if (m_startPoint.y () == rect.y ())
                    rect.setHeight (rect.width ());
                else
                    rect.setY (rect.bottom () - rect.width () + 1);
            }
            else
            {
                if (m_startPoint.x () == rect.x ())
                    rect.setWidth (rect.height ());
                else
                    rect.setX (rect.right () - rect.height () + 1);
            }
        }
        // have to maintain the centre
        else
        {
            if (rect.width () < rect.height ())
            {
                QPoint center = rect.center ();
                rect.setHeight (rect.width ());
                rect.moveCenter (center);
            }
            else
            {
                QPoint center = rect.center ();
                rect.setWidth (rect.height ());
                rect.moveCenter (center);
            }
        }
    }

    m_toolRectangleStartPoint = rect.topLeft ();
    m_toolRectangleEndPoint = rect.bottomRight ();
   
    m_toolRectangleRect = kpTool::neededRect (rect, m_pen [m_mouseButton].width ());
}

void kpToolRectangle::beginDraw ()
{
}

void kpToolRectangle::draw (const QPoint &, const QPoint &, const QRect &)
{
    applyModifiers ();
    
    viewManager ()->setTempPixmapAt (pixmap (m_mode, document (), m_toolRectangleRect,
                                     m_toolRectangleStartPoint, m_toolRectangleEndPoint,
                                     m_pen [m_mouseButton], m_brush [m_mouseButton]),
                                     m_toolRectangleRect.topLeft ());
    // TODO: for thick rect's, this'll be wrong...
    emit mouseDragged (QRect (m_startPoint, m_currentPoint));
}

void kpToolRectangle::cancelShape ()
{
#if 0
    endDraw (m_currentPoint, QRect (m_startPoint, m_currentPoint).normalize ());
    mainWindow ()->commandHistory ()->undo ();
#else
    viewManager ()->invalidateTempPixmap ();
#endif
}

void kpToolRectangle::endDraw (const QPoint &, const QRect &)
{
    applyModifiers ();
    
    // TODO: flicker
    viewManager ()->invalidateTempPixmap ();

    mainWindow ()->commandHistory ()->addCommand (new kpToolRectangleCommand
        (document (), viewManager (), m_mode, m_pen [m_mouseButton], m_brush [m_mouseButton],
        m_toolRectangleRect, m_toolRectangleStartPoint, m_toolRectangleEndPoint));
}


/*
 * kpToolRectangleCommand
 */

kpToolRectangleCommand::kpToolRectangleCommand (kpDocument *document, kpViewManager *viewManager,
                                                kpToolRectangle::Mode mode,
                                                const QPen &pen, const QBrush &brush,
                                                const QRect &rect,
                                                const QPoint &startPoint, const QPoint &endPoint)
    : m_document (document), m_viewManager (viewManager),
      m_mode (mode),
      m_pen (pen), m_brush (brush),
      m_rect (rect),
      m_startPoint (startPoint),
      m_endPoint (endPoint),
      m_oldPixmapPtr (0)
{
}

kpToolRectangleCommand::~kpToolRectangleCommand ()
{
    delete m_oldPixmapPtr;
}

// virtual
void kpToolRectangleCommand::execute ()
{
    // store Undo info
    if (!m_oldPixmapPtr)
    {
        // OPT: I can do better with no brush
        m_oldPixmapPtr = new QPixmap ();
        *m_oldPixmapPtr = m_document->getPixmapAt (m_rect);
    }
    else
        kdError () << "kpToolRectangleCommand::execute() m_oldPixmapPtr not null" << endl;

    m_document->setPixmapAt (pixmap (m_mode, m_document,
                                     m_rect, m_startPoint, m_endPoint,
                                     m_pen, m_brush),
                             m_rect.topLeft ());
}

// virtual
void kpToolRectangleCommand::unexecute ()
{
    if (m_oldPixmapPtr)
    {
        m_document->setPixmapAt (*m_oldPixmapPtr, m_rect.topLeft ());

        delete m_oldPixmapPtr;
        m_oldPixmapPtr = 0;
    }
    else
        kdError () << "kpToolRectangleCommand::unexecute() m_oldPixmapPtr null" << endl;
}

QString kpToolRectangleCommand::name () const
{
    switch (m_mode)
    {
    case kpToolRectangle::Rectangle:
        return i18n ("Rectangle");
    case kpToolRectangle::RoundedRectangle:
        return i18n ("Rounded Rectangle");
    case kpToolRectangle::Ellipse:
        return i18n ("Ellipse");
    default:
        kdError () << "kpToolRectangleCommand::name() passed unknown mode: " << int (m_mode) << endl;
        return QString::null;
    }
}

#include <kptoolrectangle.moc>
