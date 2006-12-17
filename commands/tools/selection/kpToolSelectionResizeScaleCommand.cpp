
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


#define DEBUG_KP_TOOL_SELECTION 0


#include <kpToolSelectionResizeScaleCommand.h>

#include <qapplication.h>
#include <qbitmap.h>
#include <qcursor.h>
#include <qevent.h>
#include <qmenu.h>
#include <qpainter.h>
#include <qpixmap.h>
#include <qpolygon.h>
#include <qtimer.h>

#include <kdebug.h>
#include <klocale.h>

#include <kpbug.h>
#include <kpcommandhistory.h>
#include <kpdefs.h>
#include <kpdocument.h>
#include <kpmainwindow.h>
#include <kpselection.h>
#include <kptool.h>
#include <kptooltoolbar.h>
#include <kptoolwidgetopaqueortransparent.h>
#include <kpview.h>
#include <kpviewmanager.h>


kpToolSelectionResizeScaleCommand::kpToolSelectionResizeScaleCommand (
        kpMainWindow *mainWindow)
    : kpNamedCommand (mainWindow->document ()->selection ()->isText () ?
                         i18n ("Text: Resize Box") :
                         i18n ("Selection: Smooth Scale"),
                      mainWindow),
      m_smoothScaleTimer (new QTimer (this))
{
    m_originalSelection = *selection ();

    m_newTopLeft = selection ()->topLeft ();
    m_newWidth = selection ()->width ();
    m_newHeight = selection ()->height ();

    m_smoothScaleTimer->setSingleShot (true);
    connect (m_smoothScaleTimer, SIGNAL (timeout ()),
             this, SLOT (resizeScaleAndMove ()));
}

kpToolSelectionResizeScaleCommand::~kpToolSelectionResizeScaleCommand ()
{
}


// public virtual
int kpToolSelectionResizeScaleCommand::size () const
{
    return m_originalSelection.size ();
}


// public
kpSelection kpToolSelectionResizeScaleCommand::originalSelection () const
{
    return m_originalSelection;
}


// public
QPoint kpToolSelectionResizeScaleCommand::topLeft () const
{
    return m_newTopLeft;
}

// public
void kpToolSelectionResizeScaleCommand::moveTo (const QPoint &point)
{
    if (point == m_newTopLeft)
        return;

    m_newTopLeft = point;
    selection ()->moveTo (m_newTopLeft);
}


// public
int kpToolSelectionResizeScaleCommand::width () const
{
    return m_newWidth;
}

// public
int kpToolSelectionResizeScaleCommand::height () const
{
    return m_newHeight;
}

// public
void kpToolSelectionResizeScaleCommand::resize (int width, int height,
                                                bool delayed)
{
    if (width == m_newWidth && height == m_newHeight)
        return;

    m_newWidth = width;
    m_newHeight = height;

    resizeScaleAndMove (delayed);
}


// public
void kpToolSelectionResizeScaleCommand::resizeAndMoveTo (int width, int height,
                                                         const QPoint &point,
                                                         bool delayed)
{
    if (width == m_newWidth && height == m_newHeight &&
        point == m_newTopLeft)
    {
        return;
    }

    m_newWidth = width;
    m_newHeight = height;
    m_newTopLeft = point;

    resizeScaleAndMove (delayed);
}


// protected
void kpToolSelectionResizeScaleCommand::killSmoothScaleTimer ()
{
    m_smoothScaleTimer->stop ();
}


// protected
void kpToolSelectionResizeScaleCommand::resizeScaleAndMove (bool delayed)
{
#if DEBUG_KP_TOOL_SELECTION
    kDebug () << "kpToolSelectionResizeScaleCommand::resizeScaleAndMove(delayed="
               << delayed << ")" << endl;
#endif

    killSmoothScaleTimer ();

    kpSelection newSel;

    if (selection ()->isText ())
    {
        newSel = m_originalSelection;
        newSel.textResize (m_newWidth, m_newHeight);
    }
    else
    {
        newSel = kpSelection (kpSelection::Rectangle,
            QRect (m_originalSelection.x (),
                   m_originalSelection.y (),
                   m_newWidth,
                   m_newHeight),
            kpPixmapFX::scale (*m_originalSelection.pixmap (),
                               m_newWidth, m_newHeight,
                               !delayed/*if not delayed, smooth*/),
            m_originalSelection.transparency ());

        if (delayed)
        {
            // Call self (once) with delayed==false in 200ms
            m_smoothScaleTimer->start (200/*ms*/);
        }
    }

    newSel.moveTo (m_newTopLeft);

    m_mainWindow->document ()->setSelection (newSel);
}

// protected slots
void kpToolSelectionResizeScaleCommand::resizeScaleAndMove ()
{
#if DEBUG_KP_TOOL_SELECTION
    kDebug () << "kpToolSelectionResizeScaleCommand::resizeScaleAndMove()" << endl;
#endif
    resizeScaleAndMove (false/*no delay*/);
}


// public
void kpToolSelectionResizeScaleCommand::finalize ()
{
#if DEBUG_KP_TOOL_SELECTION
    kDebug () << "kpToolSelectionResizeScaleCommand::finalize()"
               << " smoothScaleTimer->isActive="
               << m_smoothScaleTimer->isActive ()
               << endl;
#endif
    
    // Make sure the selection contains the final image and the timer won't
    // fire afterwards.
    if (m_smoothScaleTimer->isActive ())
    {
        resizeScaleAndMove ();
        Q_ASSERT (!m_smoothScaleTimer->isActive ());
    }
}


// public virtual [base kpToolResizeScaleCommand]
void kpToolSelectionResizeScaleCommand::execute ()
{
    QApplication::setOverrideCursor (Qt::WaitCursor);

    killSmoothScaleTimer ();

    resizeScaleAndMove ();

    if (m_mainWindow->tool ())
        m_mainWindow->tool ()->somethingBelowTheCursorChanged ();

    QApplication::restoreOverrideCursor ();
}

// public virtual [base kpToolResizeScaleCommand]
void kpToolSelectionResizeScaleCommand::unexecute ()
{
    QApplication::setOverrideCursor (Qt::WaitCursor);

    killSmoothScaleTimer ();

    m_mainWindow->document ()->setSelection (m_originalSelection);

    if (m_mainWindow->tool ())
        m_mainWindow->tool ()->somethingBelowTheCursorChanged ();

    QApplication::restoreOverrideCursor ();
}


#include <kpToolSelectionResizeScaleCommand.moc>
