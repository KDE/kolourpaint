
/*
   SPDX-FileCopyrightText: 2003-2007 Clarence Dang <dang@kde.org>

   SPDX-License-Identifier: BSD-2-Clause
*/

#ifndef KP_THUMBNAIL_VIEW_H
#define KP_THUMBNAIL_VIEW_H

#include "views/kpView.h"

/**
 * @short Abstract base class for all thumbnail views.
 *
 * @author Clarence Dang <dang@kde.org>
 */
class kpThumbnailView : public kpView
{
    Q_OBJECT

public:
    /**
     * Constructs a thumbnail view.
     *
     * You must call adjustEnvironment() at the end of your constructor.
     */
    kpThumbnailView(kpDocument *document,
                    kpToolToolBar *toolToolBar,
                    kpViewManager *viewManager,
                    kpView *buddyView,
                    kpViewScrollableContainer *scrollableContainer,
                    QWidget *parent);

    /**
     * Destructs this thumbnail view.
     */
    ~kpThumbnailView() override;

    /**
     * @returns the caption to display in an enclosing thumbnail window.
     */
    virtual QString caption() const = 0;

protected:
    /**
     * Sets the mask to cover the rectangle with top-left, origin() and
     * dimensions equal to or slightly less than (in case of rounding
     * error) the size of the document in view coordinates.  This ensures
     * that all pixels are initialised with either document pixels or the
     * standard widget background.
     */
    void setMaskToCoverDocument();

    /**
     * Calls adjustToEnvironment() in response to a resize event.
     *
     * Extends @ref kpView.
     */
    void resizeEvent(QResizeEvent *e) override;
};

#endif // KP_THUMBNAIL_VIEW_H
