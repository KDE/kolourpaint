
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


#define DEBUG_KP_ZOOMED_VIEW 0


#include "views/kpZoomedView.h"

#include "kpLogCategories.h"

#include "document/kpDocument.h"
#include "views/manager/kpViewManager.h"


kpZoomedView::kpZoomedView (kpDocument *document,
        kpToolToolBar *toolToolBar,
        kpViewManager *viewManager,
        kpView *buddyView,
        kpViewScrollableContainer *scrollableContainer,
        QWidget *parent)

    : kpView (document, toolToolBar, viewManager,
              buddyView,
              scrollableContainer,
              parent)
{
    // Call to virtual function - this is why the class is sealed
    adjustToEnvironment ();
}

kpZoomedView::~kpZoomedView () = default;


// public virtual [base kpView]
void kpZoomedView::setZoomLevel (int hzoom, int vzoom)
{
#if DEBUG_KP_ZOOMED_VIEW
    qCDebug(kpLogViews) << "kpZoomedView(" << name () << ")::setZoomLevel("
               << hzoom << "," << vzoom << ")" << endl;
#endif

    if (viewManager ()) {
        viewManager ()->setQueueUpdates ();
    }

    {
        kpView::setZoomLevel (hzoom, vzoom);

        adjustToEnvironment ();
    }

    if (viewManager ()) {
        viewManager ()->restoreQueueUpdates ();
    }
}


// public slot virtual [base kpView]
void kpZoomedView::adjustToEnvironment ()
{
#if DEBUG_KP_ZOOMED_VIEW
    qCDebug(kpLogViews) << "kpZoomedView(" << name () << ")::adjustToEnvironment()"
               << " doc: width=" << document ()->width ()
               << " height=" << document ()->height ()
               << endl;
#endif

    if (document ())
    {
        // TODO: use zoomedDocWidth() & zoomedDocHeight()?
        resize (static_cast<int> (transformDocToViewX (document ()->width ())),
                static_cast<int> (transformDocToViewY (document ()->height ())));
    }
}


