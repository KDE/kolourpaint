
/*
   SPDX-FileCopyrightText: 2003-2007 Clarence Dang <dang@kde.org>

   SPDX-License-Identifier: BSD-2-Clause
*/

#define DEBUG_KP_ZOOMED_THUMBNAIL_VIEW 0

#include "views/kpZoomedThumbnailView.h"

#include "document/kpDocument.h"
#include "kpLogCategories.h"
#include "views/manager/kpViewManager.h"

#include <KLocalizedString>

//--------------------------------------------------------------------------------

kpZoomedThumbnailView::kpZoomedThumbnailView(kpDocument *document,
                                             kpToolToolBar *toolToolBar,
                                             kpViewManager *viewManager,
                                             kpView *buddyView,
                                             kpViewScrollableContainer *scrollableContainer,
                                             QWidget *parent)

    : kpThumbnailView(document, toolToolBar, viewManager, buddyView, scrollableContainer, parent)
{
    // Call to virtual function - this is why the class is sealed
    adjustToEnvironment();
}

kpZoomedThumbnailView::~kpZoomedThumbnailView() = default;

// public virtual [base kpThumbnailView]
QString kpZoomedThumbnailView::caption() const
{
    return i18n("%1% - Thumbnail", zoomLevelX());
}

// public slot virtual [base kpView]
void kpZoomedThumbnailView::adjustToEnvironment()
{
#if DEBUG_KP_ZOOMED_THUMBNAIL_VIEW
    qCDebug(kpLogViews) << "kpZoomedThumbnailView(" << name() << ")::adjustToEnvironment()"
                        << " width=" << width() << " height=" << height() << endl;
#endif

    if (!document()) {
        return;
    }

#if DEBUG_KP_ZOOMED_THUMBNAIL_VIEW
    qCDebug(kpLogViews) << "\tdoc: width=" << document()->width() << " height=" << document()->height() << endl;
#endif

    if (document()->width() <= 0 || document()->height() <= 0) {
        qCCritical(kpLogViews) << "kpZoomedThumbnailView::adjustToEnvironment() doc:"
                               << " width=" << document()->width() << " height=" << document()->height();
        return;
    }

    int hzoom = qMax(1, width() * 100 / document()->width());
    int vzoom = qMax(1, height() * 100 / document()->height());

    // keep aspect ratio
    if (hzoom < vzoom) {
        vzoom = hzoom;
    } else {
        hzoom = vzoom;
    }

#if DEBUG_KP_ZOOMED_THUMBNAIL_VIEW && 1
    qCDebug(kpLogViews) << "\tproposed zoom=" << hzoom;
#endif
    if (hzoom > 100 || vzoom > 100) {
#if DEBUG_KP_ZOOMED_THUMBNAIL_VIEW && 1
        qCDebug(kpLogViews) << "\twon't magnify - setting zoom to 100%";
#endif
        hzoom = 100;
        vzoom = 100;
    }

    if (viewManager()) {
        viewManager()->setQueueUpdates();
    }

    {
        setZoomLevel(hzoom, vzoom);

        setOrigin(QPoint((width() - zoomedDocWidth()) / 2, (height() - zoomedDocHeight()) / 2));
        setMaskToCoverDocument();

        if (viewManager()) {
            viewManager()->updateView(this);
        }
    }

    if (viewManager()) {
        viewManager()->restoreQueueUpdates();
    }
}

#include "moc_kpZoomedThumbnailView.cpp"
