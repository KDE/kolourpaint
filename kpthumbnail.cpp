
/*
   Copyright (c) 2003-2004 Clarence Dang <dang@kde.org>
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


#define DEBUG_KP_THUMBNAIL 1

#include <kpthumbnail.h>

#include <qdockarea.h>
#include <qdockwindow.h>
#include <qtimer.h>

#include <kdebug.h>
#include <klocale.h>

#include <kpdefs.h>
#include <kpdocument.h>
#include <kpmainwindow.h>
#include <kptool.h>
#include <kpview.h>


// TODO: get out of the Alt+Tab list
kpThumbnail::kpThumbnail (kpMainWindow *parent, const char *name)
#if KP_IS_QT_3_3
    : QDockWindow (QDockWindow::OutsideDock, parent, name),
#else  // With Qt <3.3, OutsideDock requires parent = 0,
       // sync: make sure outside of dock
    : QDockWindow (QDockWindow::InDock, parent, name),
    #warning "Using Qt <3.3: the thumbnail will flicker more when appearing"
#endif
      m_mainWindow (parent),
      m_view (0)
{
    if (!parent)
        kdError () << "kpThumbnail::kpThumbnail() requires parent" << endl;


#if !KP_IS_QT_3_3
    if (parent)
    {
        // sync: make sure outside of dock
        parent->moveDockWindow (this, Qt::DockTornOff);
        hide ();
    }
#endif


    if (parent)
    {
        // Prevent thumbnail from docking - it's _really_ irritating otherwise
        parent->leftDock ()->setAcceptDockWindow (this, false);
        parent->rightDock ()->setAcceptDockWindow (this, false);
        parent->topDock ()->setAcceptDockWindow (this, false);
        parent->bottomDock ()->setAcceptDockWindow (this, false);
    }


    // TODO: actually work
    setMinimumSize (100, 100);

    // Enable "X" Close Button
    setCloseMode (QDockWindow::Always);

    setResizeEnabled (true);

    updateCaption ();
}

kpThumbnail::~kpThumbnail ()
{
}


// public
kpView *kpThumbnail::view () const
{
    return m_view;
}

// public
void kpThumbnail::setView (kpView *view)
{
#if DEBUG_KP_THUMBNAIL
    kdDebug () << "kpThumbnail::setView(" << view << ")" << endl;
#endif

    if (!view || m_view == view)
        return;

    m_view = view;

    setWidget (m_view);
    m_view->show ();

    updateVariableZoom ();
}


// public slot
void kpThumbnail::updateCaption ()
{
    kpView *v = view ();
    if (v)
    {
    #if DEBUG_KP_THUMBNAIL
        kdDebug () << "kpThumbnail::updateCaption() haveView; zoomLevelX="
                   << v->zoomLevelX () << endl;
    #endif
        setCaption (i18n ("%1% - Thumbnail").arg (v->zoomLevelX ()));
    }
    else
    {
    #if DEBUG_KP_THUMBNAIL
        kdDebug () << "kpThumbnail::updateCaption() no view" << endl;
    #endif
        setCaption (i18n ("Thumbnail"));
    }
}

// public slot
void kpThumbnail::updateVariableZoom ()
{
    kpView *v = view ();

#if DEBUG_KP_THUMBNAIL
    kdDebug () << "kpThumbnail::updateVariableZoom() v=" << v << endl;
#endif

    if (!v)
        return;

#if DEBUG_KP_THUMBNAIL
    kdDebug () << "\tbefore: hzoom=" << v->zoomLevelX ()
               << " vzoom=" << v->zoomLevelY ()
               << endl;
#endif

    v->slotUpdateVariableZoom ();

#if DEBUG_KP_THUMBNAIL
    kdDebug () << "\tafter: hzoom=" << v->zoomLevelX ()
               << " vzoom=" << v->zoomLevelY ()
               << endl;
#endif

    updateCaption ();
}


// public slot virtual [base QDockWindow]
void kpThumbnail::dock ()
{
#if DEBUG_KP_THUMBNAIL
    kdDebug () << "kpThumbnail::dock() CALLED - ignoring request" << endl;
#endif

    // --- ignore all requests to dock ---
}


// protected virtual [base QWidget]
void kpThumbnail::resizeEvent (QResizeEvent * /*e*/)
{
#if DEBUG_KP_THUMBNAIL
    kdDebug () << "kpThumbnail::resize(" << width () << "," << height () << ")" << endl;
#endif

    updateVariableZoom ();;

    if (m_mainWindow)
    {
        m_mainWindow->notifyThumbnailGeometryChanged ();

        if (m_mainWindow->tool ())
            m_mainWindow->tool ()->somethingBelowTheCursorChanged ();
    }
}

// protected virtual [base QWidget]
void kpThumbnail::moveEvent (QMoveEvent * /*e*/)
{
    if (m_mainWindow)
        m_mainWindow->notifyThumbnailGeometryChanged ();
}


#include <kpthumbnail.moc>
