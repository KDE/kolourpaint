
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
    kpZoomedView (kpDocument *document,
                  kpToolToolBar *toolToolBar,
                  kpViewManager *viewManager,
                  kpView *buddyView,
                  kpViewScrollableContainer *scrollableContainer,
                  QWidget *parent);

    /**
     * Destructs an unzoomed view.
     */
    ~kpZoomedView () override;


    /**
     * Extends @kpView.  Calls adjustToEnvironment().
     */
    void setZoomLevel (int hzoom, int vzoom) override;


public slots:
    /**
     * Resizes itself so that the entire document in the zoom level fits
     * almost perfectly.
     *
     * Call this if the size of the document changes.
     * Already called by setZoomLevel().
     *
     * Implements @ref kpView.
     */
    void adjustToEnvironment () override;


private:
    struct kpZoomedViewPrivate *d;
};


#endif  // KP_ZOOMED_VIEW_H
