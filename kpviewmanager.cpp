
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


#define DEBUG_KP_VIEW_MANAGER 0

#include <kdebug.h>

#include <kpdefs.h>
#include <kpdocument.h>
#include <kpmainwindow.h>
#include <kpselection.h>
#include <kpview.h>
#include <kpviewmanager.h>


kpViewManager::kpViewManager (kpMainWindow *mainWindow)
    : m_mainWindow (mainWindow),
      m_viewUnderCursor (0),
      m_selectionBorderVisible (false),
      m_selectionBorderFinished (false),
      m_tempPixmapType (NoPixmap)
{
    m_queueUpdatesCounter = m_fastUpdatesCounter =  0;
}

kpDocument *kpViewManager::document ()
{
    return m_mainWindow ? m_mainWindow->document () : 0;
}

kpViewManager::~kpViewManager ()
{
    unregisterAllViews ();
}


void kpViewManager::registerView (kpView *view)
{
#if DEBUG_KP_VIEW_MANAGER && 1
    kdDebug () << "kpViewManager::registerView (" << view << ")" << endl;
#endif
    if (view && m_views.findRef (view) < 0)
    {
    #if DEBUG_KP_VIEW_MANAGER && 1
        kdDebug () << "\tadded view" << endl;
    #endif
        m_views.append (view);
    }
    else
    {
    #if DEBUG_KP_VIEW_MANAGER && 1
        kdDebug () << "\tignored register view attempt" << endl;
    #endif
    }
}

void kpViewManager::unregisterView (kpView *view)
{
    if (view)
    {
        m_views.removeRef (view);
    }
}

void kpViewManager::unregisterAllViews ()
{
    // no autoDelete
    m_views.clear ();
}


void kpViewManager::setTempPixmapAt (const QPixmap &pixmap, const QPoint &at,
                                     enum TempPixmapType type)
{
#if DEBUG_KP_VIEW_MANAGER && 0
    kdDebug () << "kpViewManager::setTempPixmapAt (pixmap (w="
               << pixmap.width ()
               << ",h=" << pixmap.height ()
               << "), x=" << at.x ()
               << ",y=" << at.y ()
               << endl;
#endif

    bool oldPixmapActive = tempPixmapActive ();
    QRect oldPixmapRect = m_tempPixmapRect;  // only valid if oldPixmapActive

    m_tempPixmap = pixmap;
    m_tempPixmapRect = QRect (at.x (), at.y (), pixmap.width (), pixmap.height ());
    m_tempPixmapType = type;

    setQueueUpdates ();

    if (oldPixmapActive)
        updateViews (oldPixmapRect);
    updateViews (m_tempPixmapRect);

    restoreQueueUpdates ();
}

void kpViewManager::invalidateTempPixmap (const bool doUpdate)
{
    if (!tempPixmapActive ()) return;

    m_tempPixmapType = NoPixmap;

    if (doUpdate)
        updateViews (m_tempPixmapRect);
}

enum kpViewManager::TempPixmapType kpViewManager::tempPixmapType () /*const*/
{
    return m_tempPixmapType;
}

bool kpViewManager::tempPixmapActive () /*const*/
{
    return tempPixmapType () != NoPixmap;
}

bool kpViewManager::normalActive () /*const*/
{
    return tempPixmapType () == NormalPixmap;
}

bool kpViewManager::brushActive () /*const*/
{
    return tempPixmapType () == BrushPixmap;
}

bool kpViewManager::shouldBrushBeDisplayed (kpView * /*view*/) /*const*/
{
    return brushActive () && viewUnderCursor ();
}

QRect kpViewManager::tempPixmapRect () const
{
    return m_tempPixmapRect;
}

QPixmap kpViewManager::tempPixmap () const
{
    return m_tempPixmap;
}


// public
bool kpViewManager::selectionBorderVisible () const
{
    return m_selectionBorderVisible;
}

// public
void kpViewManager::setSelectionBorderVisible (bool yes)
{
    if (m_selectionBorderVisible == yes)
        return;

    m_selectionBorderVisible = yes;

    if (document () && document ()->selection ())
        updateViews (document ()->selection ()->boundingRect ());
}


// public
bool kpViewManager::selectionBorderFinished () const
{
    return m_selectionBorderFinished;
}

// public
void kpViewManager::setSelectionBorderFinished (bool yes)
{
    if (m_selectionBorderFinished == yes)
        return;

    m_selectionBorderFinished = yes;

    if (document () && document ()->selection ())
        updateViews (document ()->selection ()->boundingRect ());
}

        
void kpViewManager::setCursor (const QCursor &cursor)
{
    const int numViews = (int) m_views.count ();
    for (int i = 0; i < numViews; i++)
    {
        kpView *v = m_views.at (i);
        v->setCursor (cursor);
    }
}

void kpViewManager::unsetCursor ()
{
    const int numViews = (int) m_views.count ();
    for (int i = 0; i < numViews; i++)
    {
        kpView *v = m_views.at (i);
        v->unsetCursor ();
    }
}


kpView *kpViewManager::viewUnderCursor () /*const*/
{
    if (m_viewUnderCursor && m_views.findRef (m_viewUnderCursor) < 0)
    {
        kdError () << "kpViewManager::viewUnderCursor(): invalid view" << endl;
        m_viewUnderCursor = 0;
    }

    return m_viewUnderCursor;
}

void kpViewManager::setViewUnderCursor (kpView *view)
{
#if DEBUG_KP_VIEW_MANAGER && 0
    kdDebug () << "kpViewManager::setViewUnderCursor (" << view << ")" << endl;
#endif
    m_viewUnderCursor = view;

    updateBrushPixmap ();
}


// public
bool kpViewManager::queueUpdates () const
{
    return (m_queueUpdatesCounter > 0);
}

// public
void kpViewManager::setQueueUpdates ()
{
    m_queueUpdatesCounter++;
#if DEBUG_KP_VIEW_MANAGER && 1
    kdDebug () << "kpViewManager::setQueueUpdates() counter="
               << m_queueUpdatesCounter << endl;
#endif
}

// public
void kpViewManager::restoreQueueUpdates ()
{
    m_queueUpdatesCounter--;
#if DEBUG_KP_VIEW_MANAGER && 1
    kdDebug () << "kpViewManager::restoreQueueUpdates() counter="
               << m_queueUpdatesCounter << endl;
#endif
    if (m_queueUpdatesCounter < 0)
    {
        kdError () << "kpViewManager::restoreQueueUpdates() counter="
                   << m_queueUpdatesCounter;
    }

    if (m_queueUpdatesCounter <= 0)
    {
        const int numViews = (int) m_views.count ();
        for (int i = 0; i < numViews; i++)
        {
            kpView *v = m_views.at (i);
            v->updateQueuedArea ();
        }
    }
}


// public
bool kpViewManager::fastUpdates () const
{
    return (m_fastUpdatesCounter > 0);
}

// public
void kpViewManager::setFastUpdates ()
{
    m_fastUpdatesCounter++;
#if DEBUG_KP_VIEW_MANAGER && 1
    kdDebug () << "kpViewManager::setFastUpdates() counter="
               << m_fastUpdatesCounter << endl;
#endif
}

// public
void kpViewManager::restoreFastUpdates ()
{
    m_fastUpdatesCounter--;
#if DEBUG_KP_VIEW_MANAGER && 1
    kdDebug () << "kpViewManager::restoreFastUpdates() counter="
               << m_fastUpdatesCounter << endl;
#endif
    if (m_fastUpdatesCounter < 0)
    {
        kdError () << "kpViewManager::restoreFastUpdates() counter="
                   << m_fastUpdatesCounter;
    }
}


void kpViewManager::updateBrushPixmap ()
{
#if DEBUG_KP_VIEW_MANAGER && 0
    kdDebug () << "kpViewManager::updateBrushPixmap () viewUnderCursor="
               << viewUnderCursor () << endl;
#endif
    updateViews (tempPixmapRect ());
}


void kpViewManager::updateView (kpView *v)
{
    updateView (v, QRect (0, 0, v->width (), v->height ()));
}

void kpViewManager::updateView (kpView *v, const QRect &viewRect)
{
    if (!queueUpdates ())
    {
        if (fastUpdates ())
            v->repaint (viewRect, false/*no erase*/);
        else
            v->update (viewRect);
    }
    else
        v->addToQueuedArea (viewRect);
}

void kpViewManager::updateView (kpView *v, int x, int y, int w, int h)
{
    updateView (v, QRect (x, y, w, h));
}

void kpViewManager::updateView (kpView *v, const QRegion &viewRegion)
{
    if (!queueUpdates ())
    {
        if (fastUpdates ())
            v->repaint (viewRegion, false/*no erase*/);
        else
            v->update (viewRegion.boundingRect ());
    }
    else
        v->addToQueuedArea (viewRegion);
}


void kpViewManager::updateViews ()
{
    kpDocument *doc = document ();
    if (doc)
        updateViews (QRect (0, 0, doc->width (), doc->height ()));
}

void kpViewManager::updateViews (const QRect &docRect)
{
#if DEBUG_KP_VIEW_MANAGER && 1
    kdDebug () << "KpViewManager::updateViews (" << docRect << ")" << endl;
#endif

    const int numViews = (int) m_views.count ();
    for (int i = 0; i < numViews; i++)
    {
        kpView *view = m_views.at (i);

    #if DEBUG_KP_VIEW_MANAGER && 1
        kdDebug () << "\tupdating view " << view->name () << endl;
    #endif
        if (view->zoomLevelX () % 100 == 0 && view->zoomLevelY () % 100 == 0)
        {
        #if DEBUG_KP_VIEW_MANAGER && 1
            kdDebug () << "\t\tviewRect=" << view->zoomDocToView (docRect) << endl;
        #endif
            updateView (view, view->zoomDocToView (docRect));
        }
        else
        {
            QRect viewRect = view->zoomDocToView (docRect);

            int diff = qRound (double (QMAX (view->zoomLevelX (), view->zoomLevelY ())) / 100.0) + 1;

            QRect newRect = QRect (viewRect.x () - diff,
                                   viewRect.y () - diff,
                                   viewRect.width () + 2 * diff,
                                   viewRect.height () + 2 * diff)
                                .intersect (QRect (0, 0, view->width (), view->height ()));

        #if DEBUG_KP_VIEW_MANAGER && 1
            kdDebug () << "\t\tviewRect (+compensate)=" << newRect << endl;
        #endif
            updateView (view, newRect);
        }
    }
}

void kpViewManager::updateViews (int x, int y, int w, int h)
{
    updateViews (QRect (x, y, w, h));
}

void kpViewManager::resizeViews (int docWidth, int docHeight)
{
#if DEBUG_KP_VIEW_MANAGER && 1
    kdDebug () << "kpViewManager::resizeViews(" << docWidth << ","
               << docHeight << ")"
               << " numViews=" << m_views.count ()
               << endl;
#endif
    const int numViews = m_views.count ();;
    for (int i = 0; i < numViews; i++)
    {
        kpView *view = m_views.at (i);
        
    #if DEBUG_KP_VIEW_MANAGER && 1
        kdDebug () << "\tresize view: " << view->name ()
                   << " (variableZoom=" << view->hasVariableZoom () << ")"
                   << endl;
    #endif
        if (view->hasVariableZoom ())
        {
            view->slotUpdateVariableZoom ();
        }
        else
        {
            view->resize (view->zoomDocToViewX (docWidth),
                          view->zoomDocToViewY (docHeight));
        }
    }
}

#include <kpviewmanager.moc>
