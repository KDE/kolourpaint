
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


#define DEBUG_KP_ZOOMED_THUMBNAIL_VIEW 0


#include "views/kpZoomedThumbnailView.h"

#include "kpLogCategories.h"
#include "document/kpDocument.h"
#include "views/manager/kpViewManager.h"

#include <KLocalizedString>

//--------------------------------------------------------------------------------

kpZoomedThumbnailView::kpZoomedThumbnailView (kpDocument *document,
        kpToolToolBar *toolToolBar,
        kpViewManager *viewManager,
        kpView *buddyView,
        kpViewScrollableContainer *scrollableContainer,
        QWidget *parent )

    : kpThumbnailView (document, toolToolBar, viewManager,
                       buddyView,
                       scrollableContainer,
                       parent)
{
    // Call to virtual function - this is why the class is sealed
    adjustToEnvironment ();
}


kpZoomedThumbnailView::~kpZoomedThumbnailView () = default;


// public virtual [base kpThumbnailView]
QString kpZoomedThumbnailView::caption () const
{
    return i18n ("%1% - Thumbnail", zoomLevelX ());
}


// public slot virtual [base kpView]
void kpZoomedThumbnailView::adjustToEnvironment ()
{
#if DEBUG_KP_ZOOMED_THUMBNAIL_VIEW
    qCDebug(kpLogViews) << "kpZoomedThumbnailView(" << name ()
               << ")::adjustToEnvironment()"
               << " width=" << width ()
               << " height=" << height ()
               << endl;
#endif

    if (!document ()) {
        return;
    }

#if DEBUG_KP_ZOOMED_THUMBNAIL_VIEW
    qCDebug(kpLogViews) << "\tdoc: width=" << document ()->width ()
               << " height=" << document ()->height ()
               << endl;
#endif

    if (document ()->width () <= 0 || document ()->height () <= 0)
    {
        qCCritical(kpLogViews) << "kpZoomedThumbnailView::adjustToEnvironment() doc:"
                   << " width=" << document ()->width ()
                   << " height=" << document ()->height ();
        return;
    }


    int hzoom = qMax (1, width () * 100 / document ()->width ());
    int vzoom = qMax (1, height () * 100 / document ()->height ());

    // keep aspect ratio
    if (hzoom < vzoom) {
        vzoom = hzoom;
    }
    else {
        hzoom = vzoom;
    }

#if DEBUG_KP_ZOOMED_THUMBNAIL_VIEW && 1
    qCDebug(kpLogViews) << "\tproposed zoom=" << hzoom;
#endif
    if (hzoom > 100 || vzoom > 100)
    {
    #if DEBUG_KP_ZOOMED_THUMBNAIL_VIEW && 1
        qCDebug(kpLogViews) << "\twon't magnify - setting zoom to 100%";
    #endif
        hzoom = 100;
        vzoom = 100;
    }


    if (viewManager ()) {
        viewManager ()->setQueueUpdates ();
    }

    {
        setZoomLevel (hzoom, vzoom);

        setOrigin (QPoint ((width () - zoomedDocWidth ()) / 2,
                           (height () - zoomedDocHeight ()) / 2));
        setMaskToCoverDocument ();

        if (viewManager ()) {
            viewManager ()->updateView (this);
        }
    }

    if (viewManager ()) {
        viewManager ()->restoreQueueUpdates ();
    }
}


