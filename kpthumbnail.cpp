
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


#define DEBUG_KP_THUMBNAIL 0


#include <kpthumbnail.h>

#include <QAction>
#include <QLayout>

#include <kdebug.h>
#include <klocale.h>

#include <kpdefs.h>
#include <kpdocument.h>
#include <kpmainwindow.h>
#include <kpthumbnailview.h>
#include <kptool.h>


struct kpThumbnailPrivate
{
    kpMainWindow *mainWindow;
    kpThumbnailView *view;
};

// TODO: get out of the Alt+Tab list
// TODO: Trolltech suggests using a "tool window" instead of a QDockWidget
//       that is not allowed to dock :)  Would solve "float" button
//       problem (below) too.
kpThumbnail::kpThumbnail (kpMainWindow *parent)
    : QDockWidget (parent),
      d (new kpThumbnailPrivate ())
{
    Q_ASSERT (parent);

    d->mainWindow = parent;
    d->view = 0;


    setMinimumSize (64, 64);


    // Prevent us from docking to the mainWindow - it's _really_ irritating 
    // otherwise.
    setAllowedAreas (0);

    // No "float" button (which looks more like a maximise button)
    // since we don't allow docking.
    // TODO: Not specifying QDockWidget::DockWidgetFloatable prevents the
    //       thumbnail from begin moved.  We would rather have a useless
    //       float button that duplicates the close button.
    //setFeatures (QDockWidget::DockWidgetClosable |
    //    QDockWidget::DockWidgetMovable);

    // Float above mainWindow.
    setFloating (true);


    connect (toggleViewAction (), SIGNAL (toggled (bool)),
        this, SIGNAL (windowClosed ()));


    updateCaption ();
}

kpThumbnail::~kpThumbnail ()
{
    delete d;
}


// public
kpThumbnailView *kpThumbnail::view () const
{
    return d->view;
}

// public
void kpThumbnail::setView (kpThumbnailView *view)
{
#if DEBUG_KP_THUMBNAIL
    kDebug () << "kpThumbnail::setView(" << view << ")" << endl;
#endif

    if (d->view == view)
        return;


    if (d->view)
    {
        disconnect (d->view, SIGNAL (destroyed ()),
                    this, SLOT (slotViewDestroyed ()));
        disconnect (d->view, SIGNAL (zoomLevelChanged (int, int)),
                    this, SLOT (updateCaption ()));

        setWidget (0);
    }

    d->view = view;

    if (d->view)
    {
        connect (d->view, SIGNAL (destroyed ()),
                 this, SLOT (slotViewDestroyed ()));
        connect (d->view, SIGNAL (zoomLevelChanged (int, int)),
                 this, SLOT (updateCaption ()));

        setWidget (d->view);
        d->view->show ();
    }
    
    updateCaption ();
}


// public slot
void kpThumbnail::updateCaption ()
{
    setWindowTitle (view () ? view ()->caption () : i18n ("Thumbnail"));
}


// protected slot
void kpThumbnail::slotViewDestroyed ()
{
#if DEBUG_KP_THUMBNAIL
    kDebug () << "kpThumbnail::slotViewDestroyed()" << endl;
#endif

    d->view = 0;
    updateCaption ();
}


// protected virtual [base QWidget]
void kpThumbnail::resizeEvent (QResizeEvent *e)
{
#if DEBUG_KP_THUMBNAIL
    kDebug () << "kpThumbnail::resizeEvent(" << width ()
               << "," << height () << ")" << endl;
#endif

    QDockWidget::resizeEvent (e);

    // updateVariableZoom ();  TODO: is below a good idea since this commented out?

    if (d->mainWindow)
    {
        d->mainWindow->notifyThumbnailGeometryChanged ();

        if (d->mainWindow->tool ())
            d->mainWindow->tool ()->somethingBelowTheCursorChanged ();
    }
}

// protected virtual [base QWidget]
void kpThumbnail::moveEvent (QMoveEvent * /*e*/)
{
    if (d->mainWindow)
        d->mainWindow->notifyThumbnailGeometryChanged ();
}


#include <kpthumbnail.moc>
