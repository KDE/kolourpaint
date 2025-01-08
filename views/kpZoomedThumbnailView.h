
/*
   SPDX-FileCopyrightText: 2003-2007 Clarence Dang <dang@kde.org>

   SPDX-License-Identifier: BSD-2-Clause
*/

#ifndef KP_ZOOMED_THUMBNAIL_VIEW_H
#define KP_ZOOMED_THUMBNAIL_VIEW_H

#include "kpThumbnailView.h"

/**
 * @short Zoomed thumbnail view of a document.
 *
 * This is a zoomed thumbnail view of a document.  Unlike @ref kpZoomedView,
 * it never resizes itself.  Instead, it changes its zoom level to
 * accommodate the display of entire document in the view, while
 * maintaining aspect.
 *
 * Do not call setZoomLevel() nor setOrigin().
 *
 * This class is sealed.  Do not derive from it.
 *
 * @author Clarence Dang <dang@kde.org>
 */
/*sealed*/ class kpZoomedThumbnailView : public kpThumbnailView
{
    Q_OBJECT

public:
    /**
     * Constructs a zoomed thumbnail view.
     */
    kpZoomedThumbnailView(kpDocument *document,
                          kpToolToolBar *toolToolBar,
                          kpViewManager *viewManager,
                          kpView *buddyView,
                          kpViewScrollableContainer *scrollableContainer,
                          QWidget *parent);

    /**
     * Destructs a zoomed thumbnail view.
     */
    ~kpZoomedThumbnailView() override;

    /**
     * Implements @ref kpThumbnailView.
     */
    QString caption() const override;

public Q_SLOTS:
    /**
     * Changes its zoom level to accommodate the display of entire document
     * in the view.  It maintains aspect by changing the origin and mask.
     *
     * Call this if the size of the document changes.
     * Already called by @ref kpThumbnailView resizeEvent().
     *
     * Implements @ref kpView.
     */
    void adjustToEnvironment() override;

private:
    struct kpZoomedThumbnailViewPrivate *d;
};

#endif // KP_ZOOMED_THUMBNAIL_VIEW_H
