
/*
   SPDX-FileCopyrightText: 2003-2007 Clarence Dang <dang@kde.org>

   SPDX-License-Identifier: BSD-2-Clause
*/

#ifndef KP_UNZOOMED_THUMBNAIL_VIEW_H
#define KP_UNZOOMED_THUMBNAIL_VIEW_H

#include "views/kpThumbnailView.h"

class kpViewScrollableContainer;

/**
 * @short Unzoomed thumbnail view of a document.
 *
 * This is an unzoomed thumbnail view of a document.  Unlike
 * @ref kpZoomedThumbnailView, it never changes the zoom level.  And unlike
 * @ref kpZoomedView, it never resizes itself.  Instead, it changes its
 * origin according to the main view's scrollable container so that the
 * top-left most document pixel displayed in the scrollable container will
 * be visible.
 *
 * Do not call setZoomLevel() nor setOrigin().
 *
 * This class is sealed.  Do not derive from it.
 *
 * @author Clarence Dang <dang@kde.org>
 */
/*sealed*/ class kpUnzoomedThumbnailView : public kpThumbnailView
{
    Q_OBJECT

public:
    /**
     * Constructs an unzoomed thumbnail view.
     */
    kpUnzoomedThumbnailView(kpDocument *document,
                            kpToolToolBar *toolToolBar,
                            kpViewManager *viewManager,
                            kpView *buddyView,
                            kpViewScrollableContainer *scrollableContainer,
                            QWidget *parent);

    /**
     * Destructs an unzoomed thumbnail view.
     */
    ~kpUnzoomedThumbnailView() override;

    /**
     * Implements @ref kpThumbnailView.
     */
    QString caption() const override;

public Q_SLOTS:
    /**
     * Changes its origin according to the main view's scrollable container
     * so that the top-left most document pixel displayed in the scrollable
     * container will be visible.
     *
     * It tries to maximise the used area of this view.  Unused areas will
     * be set to the widget background thanks to the mask.
     *
     * Call this if the size of the document changes.
     * Already connected to buddyViewScrollableContainer()'s
     * contentsMoved() signal.
     * Already called by @ref kpThumbnailView resizeEvent().
     *
     * Implements @ref kpView.
     */
    void adjustToEnvironment() override;

private:
    struct kpUnzoomedThumbnailViewPrivate *d;
};

#endif // KP_UNZOOMED_THUMBNAIL_VIEW_H
