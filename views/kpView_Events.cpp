
/*
   Copyright (c) 2003-2007 Clarence Dang <dang@kde.org>
   Copyright (c) 2005 Kazuki Ohta <mover@hct.zaq.ne.jp>
   Copyright (c) 2010 Tasuku Suzuki <stasuku@gmail.com>
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


#define DEBUG_KP_VIEW 0
#define DEBUG_KP_VIEW_RENDERER ((DEBUG_KP_VIEW && 1) || 0)


#include "views/kpView.h"
#include "kpViewPrivate.h"

#if DEBUG_KP_VIEW
#include "kpLogCategories.h"
#endif

#include <QKeyEvent>
#include <QMouseEvent>

#include "tools/kpTool.h"

//---------------------------------------------------------------------

// protected virtual [base QWidget]
void kpView::mouseMoveEvent (QMouseEvent *e)
{
#if DEBUG_KP_VIEW && 0
    qCDebug(kpLogViews) << "kpView(" << objectName () << ")::mouseMoveEvent ("
               << e->x () << "," << e->y () << ")"
               << endl;
#endif

    // TODO: This is wrong if you leaveEvent the mainView by mouseMoving on the
    //       mainView, landing on top of the thumbnailView cleverly put on top
    //       of the mainView.
    setHasMouse (rect ().contains (e->pos ()));

    if (tool ()) {
        tool ()->mouseMoveEvent (e);
    }

    e->accept ();
}

// protected virtual [base QWidget]
void kpView::mousePressEvent (QMouseEvent *e)
{
#if DEBUG_KP_VIEW && 0
    qCDebug(kpLogViews) << "kpView(" << objectName () << ")::mousePressEvent ("
               << e->x () << "," << e->y () << ")"
               << endl;
#endif

    setHasMouse (true);

    if (tool ()) {
        tool ()->mousePressEvent (e);
    }

    e->accept ();
}

//---------------------------------------------------------------------

// protected virtual [base QWidget]
void kpView::mouseReleaseEvent (QMouseEvent *e)
{
#if DEBUG_KP_VIEW && 0
    qCDebug(kpLogViews) << "kpView(" << objectName () << ")::mouseReleaseEvent ("
               << e->x () << "," << e->y () << ")"
               << endl;
#endif

    setHasMouse (rect ().contains (e->pos ()));

    if (tool ()) {
        tool ()->mouseReleaseEvent (e);
    }

    e->accept ();
}

//---------------------------------------------------------------------

// public virtual [base QWidget]
void kpView::wheelEvent (QWheelEvent *e)
{
    if (tool ()) {
        tool ()->wheelEvent (e);
    }
}

//---------------------------------------------------------------------

// protected virtual [base QWidget]
void kpView::keyPressEvent (QKeyEvent *e)
{
#if DEBUG_KP_VIEW
    qCDebug(kpLogViews) << "kpView(" << objectName () << ")::keyPressEvent()" << e->text();
#endif

    if (tool ()) {
        tool ()->keyPressEvent (e);
    }

    e->accept ();
}

//---------------------------------------------------------------------

// protected virtual [base QWidget]
void kpView::keyReleaseEvent (QKeyEvent *e)
{
#if DEBUG_KP_VIEW && 0
    qCDebug(kpLogViews) << "kpView(" << objectName () << ")::keyReleaseEvent()";
#endif

    if (tool ()) {
        tool ()->keyReleaseEvent (e);
    }

    e->accept ();
}

//---------------------------------------------------------------------

// protected virtual [base QWidget]
void kpView::inputMethodEvent (QInputMethodEvent *e)
{
#if DEBUG_KP_VIEW && 1
    qCDebug(kpLogViews) << "kpView(" << objectName () << ")::inputMethodEvent()";
#endif

    if (tool ()) {
        tool ()->inputMethodEvent (e);
    }
    e->accept ();
}

// protected virtual [base QWidget]
bool kpView::event (QEvent *e)
{
#if DEBUG_KP_VIEW
    qCDebug(kpLogViews) << "kpView::event() invoking kpTool::event()";
#endif
    if (tool () && tool ()->viewEvent (e))
    {
    #if DEBUG_KP_VIEW
        qCDebug(kpLogViews) << "\tkpView::event() - tool said eat event, ret true";
    #endif
        return true;
    }

#if DEBUG_KP_VIEW
    qCDebug(kpLogViews) << "\tkpView::event() - no tool or said false, call QWidget::event()";
#endif
    return QWidget::event (e);
}


// protected virtual [base QWidget]
void kpView::focusInEvent (QFocusEvent *e)
{
#if DEBUG_KP_VIEW && 0
    qCDebug(kpLogViews) << "kpView(" << objectName () << ")::focusInEvent()";
#endif
    if (tool ()) {
        tool ()->focusInEvent (e);
    }
}

// protected virtual [base QWidget]
void kpView::focusOutEvent (QFocusEvent *e)
{
#if DEBUG_KP_VIEW && 0
    qCDebug(kpLogViews) << "kpView(" << objectName () << ")::focusOutEvent()";
#endif
    if (tool ()) {
        tool ()->focusOutEvent (e);
    }
}


// protected virtual [base QWidget]
void kpView::enterEvent (QEvent *e)
{
#if DEBUG_KP_VIEW && 0
    qCDebug(kpLogViews) << "kpView(" << objectName () << ")::enterEvent()";
#endif

    // Don't call setHasMouse(true) as it displays the brush cursor (if
    // active) when dragging open a menu and then dragging
    // past the extents of the menu due to Qt sending us an EnterEvent.
    // We're already covered by MouseMoveEvent anyway.
    //
    // But disabling this causes a more serious problem: RMB on a text
    // box and Esc.  We have no other reliable way to determine if the
    // mouse is still above the view (user could have moved mouse out
    // while RMB menu was up) and hence the cursor is not updated.
    setHasMouse (true);

    if (tool ()) {
        tool ()->enterEvent (e);
    }
}

// protected virtual [base QWidget]
void kpView::leaveEvent (QEvent *e)
{
#if DEBUG_KP_VIEW && 0
    qCDebug(kpLogViews) << "kpView(" << objectName () << ")::leaveEvent()";
#endif

    setHasMouse (false);
    if (tool ()) {
        tool ()->leaveEvent (e);
    }
}


// protected virtual [base QWidget]
void kpView::dragEnterEvent (QDragEnterEvent *)
{
#if DEBUG_KP_VIEW && 1
    qCDebug(kpLogViews) << "kpView(" << objectName () << ")::dragEnterEvent()";
#endif

    setHasMouse (true);
}

// protected virtual [base QWidget]
void kpView::dragLeaveEvent (QDragLeaveEvent *)
{
#if DEBUG_KP_VIEW && 1
    qCDebug(kpLogViews) << "kpView(" << objectName () << ")::dragLeaveEvent";
#endif

    setHasMouse (false);
}


// protected virtual [base QWidget]
void kpView::resizeEvent (QResizeEvent *e)
{
#if DEBUG_KP_VIEW && 1
    qCDebug(kpLogViews) << "kpView(" << objectName () << ")::resizeEvent("
               << e->size ()
               << " vs actual=" << size ()
               << ") old=" << e->oldSize () << endl;
#endif

    QWidget::resizeEvent (e);

    emit sizeChanged (width (), height ());
    emit sizeChanged (size ());
}
