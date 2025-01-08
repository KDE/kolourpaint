
/*
   SPDX-FileCopyrightText: 2003-2007 Clarence Dang <dang@kde.org>

   SPDX-License-Identifier: BSD-2-Clause
*/

#ifndef KP_ZOOMED_VIEW_H
#define KP_ZOOMED_VIEW_H

#include "views/kpView.h"

/**
 * @short Zoomed view of a document.  Suitable as an ordinary editing view.
 *
 * This is a zoomed view of a document.  It resizes according to the size
 * of the document and the zoom level.  Do not manually call resize() for
 * this reason.
 *
 * It is suitable as an ordinary editing view.
 *
 * Do not call setOrigin().  REFACTOR: this is bad class design - derived classes should only add functionality - not remove
 *
 * This class is sealed.  Do not derive from it.  REFACTOR: this is also bad class design
 *
 * @author Clarence Dang <dang@kde.org>
 */
/*sealed*/ class kpZoomedView : public kpView
{
    Q_OBJECT

public:
    /**
     * Constructs a zoomed view.
     */
    kpZoomedView(kpDocument *document,
                 kpToolToolBar *toolToolBar,
                 kpViewManager *viewManager,
                 kpView *buddyView,
                 kpViewScrollableContainer *scrollableContainer,
                 QWidget *parent);

    /**
     * Destructs an unzoomed view.
     */
    ~kpZoomedView() override;

    /**
     * Extends @kpView.  Calls adjustToEnvironment().
     */
    void setZoomLevel(int hzoom, int vzoom) override;

public Q_SLOTS:
    /**
     * Resizes itself so that the entire document in the zoom level fits
     * almost perfectly.
     *
     * Call this if the size of the document changes.
     * Already called by setZoomLevel().
     *
     * Implements @ref kpView.
     */
    void adjustToEnvironment() override;

private:
    struct kpZoomedViewPrivate *d;
};

#endif // KP_ZOOMED_VIEW_H
