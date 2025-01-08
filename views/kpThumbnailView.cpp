
/*
   SPDX-FileCopyrightText: 2003-2007 Clarence Dang <dang@kde.org>

   SPDX-License-Identifier: BSD-2-Clause
*/

#define DEBUG_KP_THUMBNAIL_VIEW 0

#include "views/kpThumbnailView.h"

#include "kpLogCategories.h"

kpThumbnailView::kpThumbnailView(kpDocument *document,
                                 kpToolToolBar *toolToolBar,
                                 kpViewManager *viewManager,
                                 kpView *buddyView,
                                 kpViewScrollableContainer *scrollableContainer,
                                 QWidget *parent)

    : kpView(document, toolToolBar, viewManager, buddyView, scrollableContainer, parent)
{
}

kpThumbnailView::~kpThumbnailView() = default;

// protected
void kpThumbnailView::setMaskToCoverDocument()
{
#if DEBUG_KP_THUMBNAIL_VIEW
    qCDebug(kpLogViews) << "kpThumbnailView::setMaskToCoverDocument()"
                        << " origin=" << origin() << " zoomedDoc: width=" << zoomedDocWidth() << " height=" << zoomedDocHeight() << endl;
#endif

    setMask(QRegion(QRect(origin().x(), origin().y(), zoomedDocWidth(), zoomedDocHeight())));
}

// protected virtual [base kpView]
void kpThumbnailView::resizeEvent(QResizeEvent *e)
{
#if DEBUG_KP_THUMBNAIL_VIEW
    qCDebug(kpLogViews) << "kpThumbnailView(" << name() << ")::resizeEvent()" << endl;
#endif

    // For QResizeEvent's, Qt already throws an entire widget repaint into
    // the event loop.  So eat useless update() calls that can only slow
    // things down.
    // TODO: this doesn't seem to work.
    //       Later: In Qt4, setUpdatesEnabled(true) calls update().
    const bool oldIsUpdatesEnabled = updatesEnabled();
    setUpdatesEnabled(false);

    {
        kpView::resizeEvent(e);

        adjustToEnvironment();
    }

    setUpdatesEnabled(oldIsUpdatesEnabled);
}

#include "moc_kpThumbnailView.cpp"
