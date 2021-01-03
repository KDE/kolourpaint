
/*
   Copyright (c) 2003-2007 Clarence Dang <dang@kde.org>
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


#include "views/manager/kpViewManager.h"
#include "kpViewManagerPrivate.h"

#include <QApplication>
#include <QList>
#include <QTimer>

#include "kpLogCategories.h"

#include "kpDefs.h"
#include "document/kpDocument.h"
#include "mainWindow/kpMainWindow.h"
#include "layers/tempImage/kpTempImage.h"
#include "layers/selections/text/kpTextSelection.h"
#include "tools/kpTool.h"
#include "views/kpView.h"


// public slot
bool kpViewManager::queueUpdates () const
{
    return (d->queueUpdatesCounter > 0);
}

// public slot
void kpViewManager::setQueueUpdates ()
{
    d->queueUpdatesCounter++;
#if DEBUG_KP_VIEW_MANAGER && 1
    qCDebug(kpLogViews) << "kpViewManager::setQueueUpdates() counter="
               << d->queueUpdatesCounter << endl;
#endif
}

//--------------------------------------------------------------------------------

// public slot
void kpViewManager::restoreQueueUpdates ()
{
    d->queueUpdatesCounter--;
#if DEBUG_KP_VIEW_MANAGER && 1
    qCDebug(kpLogViews) << "kpViewManager::restoreQueueUpdates() counter="
               << d->queueUpdatesCounter;
#endif
    Q_ASSERT (d->queueUpdatesCounter >= 0);

    if (d->queueUpdatesCounter == 0)
    {
      foreach (kpView *view, d->views)
        view->updateQueuedArea();
    }
}

//--------------------------------------------------------------------------------

// public slot
bool kpViewManager::fastUpdates () const
{
    return (d->fastUpdatesCounter > 0);
}

// public slot
void kpViewManager::setFastUpdates ()
{
    d->fastUpdatesCounter++;
#if DEBUG_KP_VIEW_MANAGER && 0
    qCDebug(kpLogViews) << "kpViewManager::setFastUpdates() counter="
               << d->fastUpdatesCounter << endl;
#endif
}

// public slot
void kpViewManager::restoreFastUpdates ()
{
    d->fastUpdatesCounter--;
#if DEBUG_KP_VIEW_MANAGER && 0
    qCDebug(kpLogViews) << "kpViewManager::restoreFastUpdates() counter="
               << d->fastUpdatesCounter << endl;
#endif
    Q_ASSERT (d->fastUpdatesCounter >= 0);
}


// public slot
void kpViewManager::updateView (kpView *v)
{
    updateView (v, QRect (0, 0, v->width (), v->height ()));
}

// public slot
void kpViewManager::updateView (kpView *v, const QRect &viewRect)
{
    if (!queueUpdates ())
    {
        if (fastUpdates ()) {
            v->repaint (viewRect);
        }
        else {
            v->update (viewRect);
        }
    }
    else {
        v->addToQueuedArea (viewRect);
    }
}

// public slot
void kpViewManager::updateView (kpView *v, int x, int y, int w, int h)
{
    updateView (v, QRect (x, y, w, h));
}

// public slot
void kpViewManager::updateView (kpView *v, const QRegion &viewRegion)
{
    if (!queueUpdates ())
    {
        if (fastUpdates ()) {
            v->repaint (viewRegion);
        }
        else {
            v->update (viewRegion.boundingRect ());
        }
    }
    else {
        v->addToQueuedArea (viewRegion);
    }
}


// public slot
void kpViewManager::updateViewRectangleEdges (kpView *v, const QRect &viewRect)
{
    if (viewRect.height () <= 0 || viewRect.width () <= 0) {
        return;
    }

    // Top line
    updateView (v, QRect (viewRect.x (), viewRect.y (),
                          viewRect.width (), 1));

    if (viewRect.height () >= 2)
    {
        // Bottom line
        updateView (v, QRect (viewRect.x (), viewRect.bottom (),
                              viewRect.width (), 1));

        if (viewRect.height () > 2)
        {
            // Left line
            updateView (v, QRect (viewRect.x (), viewRect.y () + 1,
                                  1, viewRect.height () - 2));

            if (viewRect.width () >= 2)
            {
                // Right line
                updateView (v, QRect (viewRect.right (), viewRect.y () + 1,
                                      1, viewRect.height () - 2));
            }
        }
    }
}

// public slot
void kpViewManager::updateViews (const QRect &docRect)
{
#if DEBUG_KP_VIEW_MANAGER && 0
    qCDebug(kpLogViews) << "kpViewManager::updateViews (" << docRect << ")";
#endif

    foreach (kpView *view, d->views)
    {
    #if DEBUG_KP_VIEW_MANAGER && 0
        qCDebug(kpLogViews) << "\tupdating view " << view->name ();
    #endif
        if (view->zoomLevelX () % 100 == 0 && view->zoomLevelY () % 100 == 0)
        {
        #if DEBUG_KP_VIEW_MANAGER && 0
            qCDebug(kpLogViews) << "\t\tviewRect=" << view->transformDocToView (docRect);
        #endif
            updateView (view, view->transformDocToView (docRect));
        }
        else
        {
            QRect viewRect = view->transformDocToView (docRect);

            int diff = qRound (double (qMax (view->zoomLevelX (), view->zoomLevelY ())) / 100.0) + 1;

            QRect newRect = QRect (viewRect.x () - diff,
                                   viewRect.y () - diff,
                                   viewRect.width () + 2 * diff,
                                   viewRect.height () + 2 * diff)
                                .intersected (QRect (0, 0, view->width (), view->height ()));

        #if DEBUG_KP_VIEW_MANAGER && 0
            qCDebug(kpLogViews) << "\t\tviewRect (+compensate)=" << newRect;
        #endif
            updateView (view, newRect);
        }
    }
}

//--------------------------------------------------------------------------------

// public slot
void kpViewManager::adjustViewsToEnvironment ()
{
#if DEBUG_KP_VIEW_MANAGER && 1
    qCDebug(kpLogViews) << "kpViewManager::adjustViewsToEnvironment()"
               << " numViews=" << d->views.count ()
               << endl;
#endif
    foreach (kpView *view, d->views)
    {
    #if DEBUG_KP_VIEW_MANAGER && 1
        qCDebug(kpLogViews) << "\tview: " << view->name ()
                   << endl;
    #endif
        view->adjustToEnvironment ();
    }
}

//--------------------------------------------------------------------------------
