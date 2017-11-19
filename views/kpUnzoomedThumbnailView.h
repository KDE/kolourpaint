
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
    kpUnzoomedThumbnailView (kpDocument *document,
            kpToolToolBar *toolToolBar,
            kpViewManager *viewManager,
            kpView *buddyView,
            kpViewScrollableContainer *scrollableContainer,
            QWidget *parent);

    /**
     * Destructs an unzoomed thumbnail view.
     */
    ~kpUnzoomedThumbnailView () override;


    /**
     * Implements @ref kpThumbnailView.
     */
    QString caption () const override;


public slots:
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
    void adjustToEnvironment () override;


private:
    struct kpUnzoomedThumbnailViewPrivate *d;
};


#endif  // KP_UNZOOMED_THUMBNAIL_VIEW_H
