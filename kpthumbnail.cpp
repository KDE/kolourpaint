
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


#define DEBUG_KP_THUMBNAIL 0


#include <kpthumbnail.h>

#include <q3dockarea.h>
#include <q3dockwindow.h>
#include <qevent.h>
#include <qtimer.h>

#include <kdebug.h>
#include <klocale.h>

#include <kpdefs.h>
#include <kpdocument.h>
#include <kpmainwindow.h>
#include <kpthumbnailview.h>
#include <kptool.h>


// TODO: get out of the Alt+Tab list
kpThumbnail::kpThumbnail (kpMainWindow *parent, const char *name)
    : Q3DockWindow (Q3DockWindow::OutsideDock, parent, name),
      m_mainWindow (parent),
      m_view (0)
{
    if (!parent)
        kError () << "kpThumbnail::kpThumbnail() requires parent" << endl;


    if (parent)
    {
        // Prevent thumbnail from docking - it's _really_ irritating otherwise
        // COMPAT: re-enable after porting to QDockWindow
#if 0
        parent->leftDock ()->setAcceptDockWindow (this, false);
        parent->rightDock ()->setAcceptDockWindow (this, false);
        parent->topDock ()->setAcceptDockWindow (this, false);
        parent->bottomDock ()->setAcceptDockWindow (this, false);
#endif
    }


    QSize layoutMinimumSize = layout () ? layout ()->minimumSize () : QSize ();
#if DEBUG_KP_THUMBNAIL
    kDebug () << "\tlayout=" << layout ()
               << " minSize=" << (layout () ? layout ()->minimumSize () : QSize ()) << endl;
    kDebug () << "\tboxLayout=" << boxLayout ()
               << " minSize=" << (boxLayout () ? boxLayout ()->minimumSize () : QSize ())
               << endl;
#endif
    if (layout ())
        layout ()->setResizeMode (QLayout::SetNoConstraint);
    setMinimumSize (qMax (layoutMinimumSize.width (), 64),
                    qMax (layoutMinimumSize.height (), 64));


    // Enable "X" Close Button
    setCloseMode (Q3DockWindow::Always);

    setResizeEnabled (true);

    updateCaption ();
}

kpThumbnail::~kpThumbnail ()
{
}


// public
kpThumbnailView *kpThumbnail::view () const
{
    return m_view;
}

// public
void kpThumbnail::setView (kpThumbnailView *view)
{
#if DEBUG_KP_THUMBNAIL
    kDebug () << "kpThumbnail::setView(" << view << ")" << endl;
#endif

    if (m_view == view)
        return;


    if (m_view)
    {
        disconnect (m_view, SIGNAL (destroyed ()),
                    this, SLOT (slotViewDestroyed ()));
        disconnect (m_view, SIGNAL (zoomLevelChanged (int, int)),
                    this, SLOT (updateCaption ()));

        boxLayout ()->remove (m_view);
    }

    m_view = view;

    if (m_view)
    {
        connect (m_view, SIGNAL (destroyed ()),
                 this, SLOT (slotViewDestroyed ()));
        connect (m_view, SIGNAL (zoomLevelChanged (int, int)),
                 this, SLOT (updateCaption ()));
        updateCaption ();

        boxLayout ()->addWidget (m_view);
        m_view->show ();
    }
}


// public slot
void kpThumbnail::updateCaption ()
{
    setCaption (view () ? view ()->caption () : i18n ("Thumbnail"));
}


// public slot virtual [base QDockWindow]
void kpThumbnail::dock ()
{
#if DEBUG_KP_THUMBNAIL
    kDebug () << "kpThumbnail::dock() CALLED - ignoring request" << endl;
#endif

    // --- ignore all requests to dock ---
}


// protected slot
void kpThumbnail::slotViewDestroyed ()
{
#if DEBUG_KP_THUMBNAIL
    kDebug () << "kpThumbnail::slotViewDestroyed()" << endl;
#endif

    m_view = 0;
    updateCaption ();
}


// protected virtual [base QWidget]
void kpThumbnail::resizeEvent (QResizeEvent *e)
{
#if DEBUG_KP_THUMBNAIL
    kDebug () << "kpThumbnail::resizeEvent(" << width ()
               << "," << height () << ")" << endl;
#endif

    Q3DockWindow::resizeEvent (e);

    // updateVariableZoom ();  TODO: is below a good idea since this commented out

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
