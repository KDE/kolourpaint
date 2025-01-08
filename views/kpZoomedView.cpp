
/*
   SPDX-FileCopyrightText: 2003-2007 Clarence Dang <dang@kde.org>

   SPDX-License-Identifier: BSD-2-Clause
*/

#define DEBUG_KP_ZOOMED_VIEW 0

#include "views/kpZoomedView.h"

#include "kpLogCategories.h"

#include "document/kpDocument.h"
#include "views/manager/kpViewManager.h"

kpZoomedView::kpZoomedView(kpDocument *document,
                           kpToolToolBar *toolToolBar,
                           kpViewManager *viewManager,
                           kpView *buddyView,
                           kpViewScrollableContainer *scrollableContainer,
                           QWidget *parent)

    : kpView(document, toolToolBar, viewManager, buddyView, scrollableContainer, parent)
{
    // Call to virtual function - this is why the class is sealed
    adjustToEnvironment();
}

kpZoomedView::~kpZoomedView() = default;

// public virtual [base kpView]
void kpZoomedView::setZoomLevel(int hzoom, int vzoom)
{
#if DEBUG_KP_ZOOMED_VIEW
    qCDebug(kpLogViews) << "kpZoomedView(" << name() << ")::setZoomLevel(" << hzoom << "," << vzoom << ")" << endl;
#endif

    if (viewManager()) {
        viewManager()->setQueueUpdates();
    }

    {
        kpView::setZoomLevel(hzoom, vzoom);

        adjustToEnvironment();
    }

    if (viewManager()) {
        viewManager()->restoreQueueUpdates();
    }
}

// public slot virtual [base kpView]
void kpZoomedView::adjustToEnvironment()
{
#if DEBUG_KP_ZOOMED_VIEW
    qCDebug(kpLogViews) << "kpZoomedView(" << name() << ")::adjustToEnvironment()"
                        << " doc: width=" << document()->width() << " height=" << document()->height() << endl;
#endif

    if (document()) {
        // TODO: use zoomedDocWidth() & zoomedDocHeight()?
        resize(static_cast<int>(transformDocToViewX(document()->width())), static_cast<int>(transformDocToViewY(document()->height())));
    }
}

#include "moc_kpZoomedView.cpp"
