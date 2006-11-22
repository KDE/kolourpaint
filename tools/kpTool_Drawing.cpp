
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


#define DEBUG_KP_TOOL 0


#include <kptool.h>
#include <kpToolPrivate.h>

#include <limits.h>

#include <qapplication.h>
#include <qclipboard.h>
#include <qcursor.h>
#include <qevent.h>
#include <qlayout.h>
#include <qpainter.h>
#include <qpixmap.h>

#include <kapplication.h>
#include <kdebug.h>
#include <kiconloader.h>
#include <klocale.h>
#include <kmessagebox.h>

#include <kpbug.h>
#include <kpcolor.h>
#include <kpcolortoolbar.h>
#include <kpdefs.h>
#include <kpmainwindow.h>
#include <kppixmapfx.h>
#include <kptoolaction.h>
#include <kptooltoolbar.h>
#include <kpview.h>
#include <kpviewmanager.h>
#include <kactioncollection.h>


void kpTool::beginInternal ()
{
#if DEBUG_KP_TOOL
    kDebug () << "kpTool::beginInternal()" << endl;
#endif

    if (!d->began)
    {
        // clear leftover statusbar messages
        setUserMessage ();
        m_currentPoint = currentPoint ();
        m_currentViewPoint = currentPoint (false/*view point*/);
        setUserShapePoints (m_currentPoint);

        // TODO: Audit all the code in this file - states like "d->began" &
        //       "d->beganDraw" should be set before calling user func.
        //       Also, m_currentPoint should be more frequently initialised.

        // call user virtual func
        begin ();

        // we've starting using the tool...
        d->began = true;

        // but we haven't started drawing with it
        d->beganDraw = false;


        uint keyState = QApplication::keyboardModifiers ();

        m_shiftPressed = (keyState & Qt::ShiftModifier);
        m_controlPressed = (keyState & Qt::ControlModifier);

        // TODO: Can't do much about ALT - unless it's always KApplication::Modifier1?
        //       Ditto for everywhere else where I set SHIFT & CTRL but not alt.  COMPAT: supported by Qt
        m_altPressed = false;
    }
}

void kpTool::endInternal ()
{
    if (d->began)
    {
        // before we can stop using the tool, we must stop the current drawing operation (if any)
        if (hasBegunShape ())
            endShapeInternal (m_currentPoint, kpBug::QRect_Normalized (QRect (m_startPoint, m_currentPoint)));

        // call user virtual func
        end ();

        // clear leftover statusbar messages
        setUserMessage ();
        setUserShapePoints (currentPoint ());

        // we've stopped using the tool...
        d->began = false;

        // and so we can't be drawing with it
        d->beganDraw = false;

        if (d->mainWindow)
        {
            kpToolToolBar *tb = d->mainWindow->toolToolBar ();
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
#if DEBUG_KP_TOOL
    kDebug () << "kpTool::begin() base implementation" << endl;
#endif
}

// virtual
void kpTool::end ()
{
#if DEBUG_KP_TOOL
    kDebug () << "kpTool::end() base implementation" << endl;
#endif
}


bool kpTool::hasBegun () const { return d->began; }

bool kpTool::hasBegunDraw () const { return d->beganDraw; }

// virtual
bool kpTool::hasBegunShape () const { return hasBegunDraw (); }


void kpTool::beginDrawInternal ()
{
    if (!d->beganDraw)
    {
        beginDraw ();

        d->beganDraw = true;
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
#if DEBUG_KP_TOOL
    kDebug () << "kpTool::hover" << point
               << " base implementation"
               << endl;
#endif

    setUserShapePoints (point);
}

// virtual
void kpTool::globalDraw ()
{
}

// virtual
void kpTool::reselect ()
{
#if DEBUG_KP_TOOL
    kDebug () << "kpTool::reselect() base implementation" << endl;
#endif
}


// virtual
void kpTool::draw (const QPoint &, const QPoint &, const QRect &)
{
}

// also called by kpView
void kpTool::cancelShapeInternal ()
{
    if (hasBegunShape ())
    {
        d->beganDraw = false;
        cancelShape ();
        m_viewUnderStartPoint = 0;

        emit cancelledShape (viewUnderCursor () ? m_currentPoint : KP_INVALID_POINT);

        if (viewUnderCursor ())
            hover (m_currentPoint);
        else
        {
            m_currentPoint = KP_INVALID_POINT;
            m_currentViewPoint = KP_INVALID_POINT;
            hover (m_currentPoint);
        }

        if (returnToPreviousToolAfterEndDraw ())
        {
            kpToolToolBar *tb = mainWindow ()->toolToolBar ();
            
            // (don't end up with no tool selected)
            if (tb->previousTool ())
            {
                // endInternal() will be called by kpMainWindow (thanks to this line)
                // so we won't have the view anymore
                tb->selectPreviousTool ();
            }
        }
    }
}

// virtual
void kpTool::cancelShape ()
{
    kWarning () << "Tool cannot cancel operation!" << endl;
}

void kpTool::releasedAllButtons ()
{
}

void kpTool::endDrawInternal (const QPoint &thisPoint, const QRect &normalizedRect,
                              bool wantEndShape)
{
#if DEBUG_KP_TOOL && 1
    kDebug () << "kpTool::endDrawInternal() wantEndShape=" << wantEndShape << endl;
#endif

    if (wantEndShape && !hasBegunShape ())
        return;
    else if (!wantEndShape && !hasBegunDraw ())
        return;

    d->beganDraw = false;

    if (wantEndShape)
    {
    #if DEBUG_KP_TOOL && 0
        kDebug () << "\tcalling endShape()" << endl;
    #endif
        endShape (thisPoint, normalizedRect);
    }
    else
    {
    #if DEBUG_KP_TOOL && 0
        kDebug () << "\tcalling endDraw()" << endl;
    #endif
        endDraw (thisPoint, normalizedRect);
    }
    m_viewUnderStartPoint = 0;

    emit endedDraw (m_currentPoint);
    if (viewUnderCursor ())
        hover (m_currentPoint);
    else
    {
        m_currentPoint = KP_INVALID_POINT;
        m_currentViewPoint = KP_INVALID_POINT;
        hover (m_currentPoint);
    }

    if (returnToPreviousToolAfterEndDraw ())
    {
        kpToolToolBar *tb = mainWindow ()->toolToolBar ();
        
        // (don't end up with no tool selected)
        if (tb->previousTool ())
        {
            // endInternal() will be called by kpMainWindow (thanks to this line)
            // so we won't have the view anymore
            tb->selectPreviousTool ();
        }
    }
}

// private
void kpTool::endShapeInternal (const QPoint &thisPoint, const QRect &normalizedRect)
{
    endDrawInternal (thisPoint, normalizedRect, true/*end shape*/);
}

// virtual
void kpTool::endDraw (const QPoint &, const QRect &)
{
}
