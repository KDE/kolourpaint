
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
#include <qlayout.h>
#include <qpainter.h>

#include <kdebug.h>

#include <kpcolortoolbar.h>
#include <kpmainwindow.h>
#include <kptooltoolbar.h>
#include <kpview.h>
#include <kpviewmanager.h>

#include <kpdefs.h>
#include <kptool.h>

#define DEBUG_KPTOOL 1

//
// kpTool
//

kpTool::kpTool (const QString &text, const QString &description,
                kpMainWindow *mainWindow, const char *name)
    : m_beganDraw (false),
      m_text (text), m_description (description), m_name (name),
      m_mainWindow (mainWindow),
      m_began (false),
      m_viewUnderStartPoint (0)
{
    m_shiftPressed = m_controlPressed = m_altPressed = false;
}

kpTool::~kpTool ()
{
    // before destructing, stop using the tool
    if (m_began)
        endInternal ();
}

// static
QRect kpTool::neededRect (const QRect &rect, int lineWidth)
{
    int x1, y1, x2, y2;
    rect.coords (&x1, &y1, &x2, &y2);

    return QRect (QPoint (x1 - lineWidth + 1, y1 - lineWidth + 1),
                  QPoint (x2 + lineWidth - 1, y2 + lineWidth - 1));
}

// static
QPixmap kpTool::neededPixmap (const QPixmap &pixmap, const QRect &boundingRect)
{
    QPixmap newPixmap (boundingRect.width (), boundingRect.height ());
    QPainter painter;

    painter.begin (&newPixmap);
    painter.drawPixmap (QPoint (0, 0), pixmap, boundingRect);
    painter.end ();

    return newPixmap;
}

void kpTool::beginInternal ()
{
    kdDebug () << "kpTool::beginInternal()" << endl;

    if (!m_began)
    {
        // call user virtual func
        begin ();

        // we've starting using the tool...
        m_began = true;

        // but we haven't started drawing with it
        m_beganDraw = false;
    }
}

void kpTool::endInternal ()
{
    if (m_began)
    {
        // before we can stop using the tool, we must stop the current drawing operation (if any)
        if (m_beganDraw)
            endDrawInternal (m_currentPoint, QRect (m_startPoint, m_currentPoint).normalize ());

        // call user virtual func
        end ();

        // we've stopped using the tool...
        m_began = false;

        // and so we can't be drawing with it
        m_beganDraw = false;

        if (m_mainWindow)
        {
            kpToolToolBar *tb = m_mainWindow->toolToolBar ();
            if (tb)
            {
                tb->hideAllToolWidgets ();
            }
        }

    }
}

// virtual
void kpTool::begin ()
{
#if DEBUG_KPTOOL
    kdDebug () << "kpTool::begin() base implementation" << endl;
#endif
}

// virtual
void kpTool::end ()
{
#if DEBUG_KPTOOL
    kdDebug () << "kpTool::end() base implementation" << endl;
#endif
}

void kpTool::beginDrawInternal ()
{
    if (!m_beganDraw)
    {
        beginDraw ();

        m_beganDraw = true;
        emit beganDraw (m_currentPoint);
    }
}

// virtual
void kpTool::beginDraw ()
{
}

// virtual
void kpTool::hover (const QPoint &point)
{
    emit mouseMoved (point);
}

// virtual
void kpTool::draw (const QPoint &, const QPoint &, const QRect &)
{
}

// also called by kpView
void kpTool::cancelDrawInternal ()
{
    if (m_beganDraw)
    {
        cancelDraw ();
        m_viewUnderStartPoint = 0;
        m_beganDraw = false;

        if (returnToPreviousToolAfterEndDraw ())
        {
            // endInternal() will be called by kpMainWindow (thanks to this line)
            // so we won't have the view anymore
            mainWindow ()->switchToPreviousTool ();
        }
    }
}

// virtual
void kpTool::cancelDraw ()
{
    kdWarning () << "Tool cannot cancel operation!" << endl;
}

void kpTool::endDrawInternal (const QPoint &thisPoint, const QRect &normalizedRect)
{
    if (m_beganDraw)
    {
        endDraw (thisPoint, normalizedRect);
        m_viewUnderStartPoint = 0;
        m_beganDraw = false;

        emit endedDraw (m_currentPoint);

        if (returnToPreviousToolAfterEndDraw ())
        {
            // endInternal() will be called by kpMainWindow (thanks to this line)
            // so we won't have the view anymore
            mainWindow ()->switchToPreviousTool ();
        }
    }
}

// virtual
void kpTool::endDraw (const QPoint &, const QRect &)
{
}

kpMainWindow *kpTool::mainWindow () const
{
    return m_mainWindow;
}

kpDocument *kpTool::document () const
{
    return m_mainWindow ? m_mainWindow->document () : 0;
}

kpView *kpTool::viewUnderCursor () const
{
    return viewManager ()->viewUnderCursor ();
}

kpViewManager *kpTool::viewManager () const
{
    return m_mainWindow ? m_mainWindow->viewManager () : 0;
}

kpToolToolBar *kpTool::toolToolBar () const
{
    return m_mainWindow ? m_mainWindow->toolToolBar () : 0;
}

QColor kpTool::color (int which) const
{
    if (m_mainWindow)
        return m_mainWindow->colorToolBar ()->color (which);
    else
    {
        kdError () << "kpTool::color () called without mainWindow" << endl;
        return Qt::black;
    }
}


bool kpTool::currentPointNextToLast () const
{
    if (m_lastPoint == QPoint (-1, -1))
        return true;

    int dx = kAbs (m_currentPoint.x () - m_lastPoint.x ());
    int dy = kAbs (m_currentPoint.y () - m_lastPoint.y ());

    return (dx <= 1 && dy <= 1);
}

bool kpTool::currentPointCardinallyNextToLast () const
{
    if (m_lastPoint == QPoint (-1, -1))
        return true;

    int dx = kAbs (m_currentPoint.x () - m_lastPoint.x ());
    int dy = kAbs (m_currentPoint.y () - m_lastPoint.y ());

    return (dx + dy <= 1);
}

KCommandHistory *kpTool::commandHistory () const
{
    return m_mainWindow ? m_mainWindow->commandHistory () : 0;
}

void kpTool::mousePressEvent (QMouseEvent *e)
{
#if DEBUG_KPTOOL
    kdDebug () << "kpTool::mousePressEvent pos=" << e->pos ()
               << " btnStateAfter=" << (int) e->stateAfter ()
               << " beganDraw=" << m_beganDraw << endl;
#endif

    // state of all the buttons - not just the one that triggered the event (button())
    Qt::ButtonState buttonState = e->stateAfter ();
    int mb = mouseButton (buttonState);

    if (mb == -1 && !m_beganDraw) return; // ignore

    if (m_beganDraw)
    {
        if (mb == -1 || mb != m_mouseButton)
        {
        #if DEBUG_KPTOOL
            kdDebug () << "\tCancelling operation as " << mb << " == -1 or != " << m_mouseButton << endl;
        #endif

            kpView *view = viewUnderStartPoint ();
            if (!view)
            {
                kdError () << "kpTool::mousePressEvent() cancel without a view under the start point!" << endl;
            }

            // if we get a mousePressEvent when we're drawing, then the other
            // mouse button must have been pressed
            m_currentPoint = view ? view->zoomViewToDoc (e->pos ()) : QPoint (-1, -1);
            cancelDrawInternal ();
        }

        return;
    }

    kpView *view = viewUnderCursor ();
    if (!view)
    {
        kdError () << "kpTool::mousePressEvent() without a view under the cursor!" << endl;
    }

#if DEBUG_KPTOOL
    if (view)
        kdDebug () << "\tview=" << view->name () << endl;
#endif


    // let user know what mouse button is being used for entire draw
    m_mouseButton = mouseButton (buttonState);

    m_startPoint = m_currentPoint = view ? view->zoomViewToDoc (e->pos ()) : QPoint (-1, -1);
    m_viewUnderStartPoint = view;
    m_lastPoint = QPoint (-1, -1);

#if DEBUG_KPTOOL
    kdDebug () << "\tBeginning draw @ " << m_currentPoint << endl;
#endif

    beginDrawInternal ();

    draw (m_currentPoint, m_lastPoint, QRect (m_currentPoint, m_currentPoint));
    m_lastPoint = m_currentPoint;
}

void kpTool::mouseMoveEvent (QMouseEvent *e)
{
#if DEBUG_KPTOOL
    kdDebug () << "kpTool::mouseMoveEvent pos=" << e->pos ()
               << " btnStateAfter=" << (int) e->stateAfter () << endl;
#endif
    if (m_beganDraw)
    {
        kpView *view = viewUnderStartPoint ();
        if (!view)
        {
            kdError () << "kpTool::mouseMoveEvent() without a view under the start point!" << endl;
            return;
        }

        m_currentPoint = view->zoomViewToDoc (e->pos ());

        kdDebug () << "\tDraw!" << endl;
        draw (m_currentPoint, m_lastPoint, QRect (m_startPoint, m_currentPoint).normalize ());
        m_lastPoint = m_currentPoint;
    }
    else
    {
        kpView *view = viewUnderCursor ();
        if (!view)  // possible if cancelDraw()'ed but still holding down initial mousebtn
            return;

        m_currentPoint = view->zoomViewToDoc (e->pos ());
        hover (m_currentPoint);
    }
}

void kpTool::mouseReleaseEvent (QMouseEvent *e)
{
#if DEBUG_KPTOOL
    kdDebug () << "kpTool::mouseReleaseEvent pos=" << e->pos ()
               << " btnStateAfter=" << (int) e->stateAfter () << endl;
#endif

    if (m_beganDraw)  // didn't cancelDraw()
    {
        kpView *view = viewUnderStartPoint ();
        if (!view)
        {
            kdError () << "kpTool::mouseReleaseEvent() without a view under the start point!" << endl;
            return;
        }

        m_currentPoint = view ? view->zoomViewToDoc (e->pos ()) : QPoint (-1, -1);
        endDrawInternal (m_currentPoint, QRect (m_startPoint, m_currentPoint).normalize ());
    }
}

void kpTool::keyPressEvent (QKeyEvent *e)
{
    int dx = 0, dy = 0;

    switch (e->key ())
    {
    case Qt::Key_Alt:
    case Qt::Key_Shift:
    case Qt::Key_Control:
        setShiftPressed (e->stateAfter () & Qt::ShiftButton);
        setControlPressed (e->stateAfter () & Qt::ControlButton);
        setAltPressed (e->stateAfter () & Qt::AltButton);

        e->accept ();
        break;

    /*
     * QCursor::setPos conveniently causes mouseMoveEvents :)
     */

    case Qt::Key_Home:     dx = -1, dy = -1;    break;
    case Qt::Key_Up:                dy = -1;    break;
    case Qt::Key_PageUp:   dx = +1, dy = -1;    break;

    case Qt::Key_Left:     dx = -1;             break;
    case Qt::Key_Right:    dx = +1;             break;

    case Qt::Key_End:      dx = -1, dy = +1;    break;
    case Qt::Key_Down:              dy = +1;    break;
    case Qt::Key_PageDown: dx = +1, dy = +1;    break;

    case Qt::Key_Enter:
    case Qt::Key_Insert:
    {
        kpView *view = viewUnderCursor (); // TODO: wrong for dragging lines outside of view (for e.g.)
    // TODO: what's the 5 key called?
        if (view)
        {
            QMouseEvent me (QEvent::MouseButtonPress,
                            view->mapFromGlobal (QCursor::pos ()),
                            Qt::LeftButton,
                            0);
            mousePressEvent (&me);
            e->accept ();
        }
        break;
    }
    default:
        e->ignore ();
        break;
    }

    kpView *view = viewUnderCursor ();
    if (view && (dx || dy))
    {
        QPoint oldPoint = view->mapFromGlobal (QCursor::pos ());

        int x = QMIN (QMAX (oldPoint.x () + dx, 0), view->width () - 1);
        int y = QMIN (QMAX (oldPoint.y () + dy, 0), view->height () - 1);

        QCursor::setPos (view->mapToGlobal (QPoint (x, y)));
        e->accept ();
    }
}

void kpTool::keyReleaseEvent (QKeyEvent *e)
{
    switch (e->key ())
    {
    case Qt::Key_Alt:
    case Qt::Key_Shift:
    case Qt::Key_Control:
        setShiftPressed (e->stateAfter () & Qt::ShiftButton);
        setControlPressed (e->stateAfter () & Qt::ControlButton);
        setAltPressed (e->stateAfter () & Qt::AltButton);

        e->accept ();
        break;

    case Qt::Key_Escape:
        if (hasBegunDraw ())
            cancelDrawInternal ();

        e->accept ();
        break;

    case Qt::Key_Enter:
    case Qt::Key_Insert:
    {
        kpView *view = viewUnderCursor ();
    // TODO: what's the 5 key called?
        if (view)
        {
            QMouseEvent me (QEvent::MouseButtonRelease,
                            view->mapFromGlobal (QCursor::pos ()),
                            Qt::LeftButton,
                            Qt::LeftButton);
            mouseReleaseEvent (&me);
            e->accept ();
        }
        break;
    }}
}

void kpTool::setShiftPressed (bool pressed)
{
    m_shiftPressed = pressed;
    if (m_beganDraw && careAboutModifierState ())
        draw (m_currentPoint, m_lastPoint, QRect (m_startPoint, m_currentPoint).normalize ());
}

void kpTool::setControlPressed (bool pressed)
{
    m_controlPressed = pressed;
    if (m_beganDraw && careAboutModifierState ())
        draw (m_currentPoint, m_lastPoint, QRect (m_startPoint, m_currentPoint).normalize ());
}

void kpTool::setAltPressed (bool pressed)
{
    m_altPressed = pressed;
    if (m_beganDraw && careAboutModifierState ())
        draw (m_currentPoint, m_lastPoint, QRect (m_startPoint, m_currentPoint).normalize ());
}

void kpTool::focusInEvent (QFocusEvent *)
{
}

void kpTool::focusOutEvent (QFocusEvent *)
{
#if DEBUG_KPTOOL
    kdDebug () << "kpTool::focusOutEvent() beganDraw=" << m_beganDraw << endl;
#endif

    if (m_beganDraw)
        endDrawInternal (m_currentPoint, QRect (m_startPoint, m_currentPoint).normalize ());
}

void kpTool::enterEvent (QEvent *)
{
}

void kpTool::leaveEvent (QEvent *)
{
    // if we haven't started drawing (e.g. dragging a rectangle)...
    if (!m_beganDraw)
        emit mouseMoved (QPoint (-1, -1));
}

// static
int kpTool::mouseButton (const Qt::ButtonState &buttonState)
{
    // we have nothing to do with mid-buttons
    if (buttonState & Qt::MidButton)
        return -1;

    // both left & right together is quite meaningless...
    Qt::ButtonState bothButtons = (Qt::ButtonState) (Qt::LeftButton | Qt::RightButton);
    if ((buttonState & bothButtons) == bothButtons)
        return -1;

    if (buttonState & Qt::LeftButton)
        return 0;
    else if (buttonState & Qt::RightButton)
        return 1;
    else
        return -1;
}

#include <kptool.moc>
